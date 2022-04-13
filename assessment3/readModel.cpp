
#include "readModel.h"

using namespace std;

template<typename K, typename V>
static map<V, K> reverse_map(const map<K, V>& m) {
	map<V, K> r;
	for (const auto& kv : m)
		r[kv.second] = kv.first;
	return r;
}

enum ObjKeywords 
{ 
	NULL_KEYWORD,
	COMMENT,
	MATERIAL_FILE,
	OBJECT, 
	GROUP,
	VERTEX, 
	TEXTURE_MAP, 
	NORMAL,
	USE_MATERIAL,
	SMOOTH_SHADDING,
	FACE_INDEX,
};

enum class FaceType
{
	NO_TYPE = 0,
	V = 1,
	V_VT_VN = 2,
	V_VN = 3,
	V_VT = 4
};

static map< ObjKeywords, const string > objKeyMap = {
	{	COMMENT,			"#"			},
	{	MATERIAL_FILE,		"mtllib"	},
	{	OBJECT,				"o"			},
	{	GROUP,				"g"			},
	{	VERTEX,				"v"			},
	{	TEXTURE_MAP,		"vt"		},
	{	NORMAL,				"vn"		},
	{	USE_MATERIAL,		"usemtl"	},
	{	SMOOTH_SHADDING,	"s"			},
	{	FACE_INDEX,			"f"			},
	//{	FaceDelimiter1,	"/"			},
	//{	FaceDelimiter2,	"//"		}
};
static map< const string, ObjKeywords> objValMap = reverse_map(objKeyMap);


// =============== Main Functions ==================

WholeObj ObjFileReader::readObj(const char* filename) 
{
	
	// input
	ifstream inputFile;
	inputFile.open(filename);

	// intermediate data
	char delim = ' ';
	char faceDelim = '/';

	WholeObj data;

	string line = "";
	FaceType faceType = FaceType::NO_TYPE;
	FaceType detectedFaceType = FaceType::NO_TYPE;
	Pair tempP;
	Triple tempT;
	vector<unsigned int> tempUI;
	
	int lineCount = 0;
	int objectCount = 0;

	while (getline(inputFile, line))
	{		
		stringstream inputString(line);

		string subStr = "";		// sub string delimited with delim
		string fsubStr = "";	// sub string delimited with faceDelim

		// first element of the line
		getline(inputString, subStr, delim);
		ObjKeywords key = objValMap[(const string)subStr];
		
		switch (key)
		{
		case NULL_KEYWORD:
			throw invalid_argument(
				errString("ObjFileReader::Not Supported Keyword line", 
					filename, line, lineCount));
			break;

		case COMMENT: // do nothing
			break;
			
		case MATERIAL_FILE:
			parse1s(inputString, delim, subStr);
			data.mtl_file = subStr;	
			parseEOL(inputString, delim, 
				errString("ObjFileReader::Too many paths for a single material file", 
					filename, line, lineCount));
			break;

		case OBJECT:
			data.subObjects.push_back(SubObj());		
			parse1s(inputString, delim, subStr);
			data.subObjects.back().modelObjectName = subStr;
			parseEOL(inputString, delim,
				errString("ObjFileReader::Too many names for a single object", 
					filename, line, lineCount));
			faceType = FaceType::NO_TYPE; // reset face type for parsing new object face later on
			break;

		case GROUP:
			throw invalid_argument(
				errString("ObjFileReader::Object grouping is not supported",
					filename, line, lineCount));
			break;

		case VERTEX:			
			tempT = parse3f(
				inputString, delim, 
				errString("ObjFileReader::Length of vertex coordinate is not 3", 
					filename, line, lineCount));
			data.vertices.push_back(tempT);
			parseEOL(
				inputString, delim,
				errString("ObjFileReader::More than 3 points in a vertex", 
					filename, line, lineCount));
			break;

		case TEXTURE_MAP:			
			tempP = parse2f(
				inputString, delim, 
				errString("ObjFileReader::Length of texture coordinate is not 2",
					filename, line, lineCount));
			data.texCoords.push_back(tempP);
			parseEOL(
				inputString, delim, 
				errString("ObjFileReader::More than 2 points in texture coordinate", 
					filename, line, lineCount));
			break;

		case NORMAL:			
			tempT = parse3f(
				inputString, delim, 
				errString("ObjFileReader::Length of normals is not 3", 
					filename, line, lineCount));
			data.normals.push_back(tempT);
			parseEOL(
				inputString, delim,
				errString("ObjFileReader::More than 3 values in a normals",
					filename, line, lineCount));
			break;

		case USE_MATERIAL:
			parse1s(inputString, delim, subStr);
			data.subObjects.back().useMaterial = subStr;			
			parseEOL(
				inputString, delim,
				errString("ObjFileReader::Use too many materials",
					filename, line, lineCount));
			break;

		case SMOOTH_SHADDING:
			parse1s(inputString, delim, subStr);
			data.subObjects.back().smoothShadding = subStr;
			parseEOL(
				inputString, delim,
				errString("ObjFileReader::Too many arguments in smooth shadding",
					filename, line, lineCount));
			break;

		case FACE_INDEX:
			for (int i = 0; i < 3; i++)
			{
				parse1s(inputString, delim, subStr);
				stringstream subFaceStr(subStr);
				detectedFaceType = parseSubFace(subFaceStr, faceDelim, tempUI,
					errString("ObjFileReader::Invalid Face Indices Type",
						filename, line, lineCount));
				
				if (faceType == FaceType::NO_TYPE)
				{
					faceType = detectedFaceType;
				}
				else 
				{
					if (detectedFaceType != faceType)
					{
						throw invalid_argument(
							errString("ObjFileReader::Inconsistent Face Indices Type",
								filename, line, lineCount));
					}
				}

				data.subObjects.back().verticesIdx.push_back(tempUI[0]);
				switch (faceType)
				{
				case FaceType::V_VT_VN:
					data.subObjects.back().textureMapIdx.push_back(tempUI[1]);
					data.subObjects.back().normalsIdx.push_back(tempUI[2]);
					break;
				case FaceType::V_VN:
					data.subObjects.back().normalsIdx.push_back(tempUI[1]);
					break;
				case FaceType::V_VT:
					data.subObjects.back().textureMapIdx.push_back(tempUI[1]);
					break;
				}

			}
			parseEOL(inputString, delim,
				errString("ObjFileReader::More than 3 sets of indices, please triangulate object",
					filename, line, lineCount));
			break;
		}

		line = "";
		lineCount++;
	}
	inputFile.close();

	expandVertices(data);

	return data;
}



void ObjFileReader::expandVertices(WholeObj& data)
{
	for (int i = 0; i < data.subObjects.size(); i++)
	{
		SubObj& subObjI = data.subObjects[i];

		vector<Triple>& ver = data.vertices;
		vector<Pair>& tex = data.texCoords;
		vector<Triple>& nor = data.normals;

		vector<unsigned int>& vId = subObjI.verticesIdx;
		vector<unsigned int>& tId = subObjI.textureMapIdx;
		vector<unsigned int>& nId = subObjI.normalsIdx;
		
		vector<float>& exVer = subObjI.expandedVertices;
						
		for (int j = 0; j < vId.size(); j++)
		{
			// sequentially index index of vertex and use it to index vertex
			exVer.push_back(ver[vId[j]-1].x);
			exVer.push_back(ver[vId[j]-1].y);
			exVer.push_back(ver[vId[j]-1].z);

			// sequentially index index of texCoord and use it to index textureCoord
			exVer.push_back(tex[tId[j]-1].x);
			exVer.push_back(tex[tId[j]-1].y);

			// sequentially index index of normals and use it to index normals
			exVer.push_back(nor[nId[j]-1].x);
			exVer.push_back(nor[nId[j]-1].y);
			exVer.push_back(nor[nId[j]-1].z);
		}
	}
};



// ========== Auxilliary Functions =============

// == string parser ==

bool ObjFileReader::parse1s(stringstream& ss, char delim, string& outputString)
{
	return (bool)getline(ss, outputString, delim);
}

// == float parser ==

float ObjFileReader::parse1f(stringstream& ss, char delim, string errStr)
{
	string out = "";
	getline(ss, out, delim);
	if (out.size() == 0) throw invalid_argument(errStr);
	return stof(out.c_str());
}

Pair ObjFileReader::parse2f(stringstream& ss, char delim, string errStr) 
{
	Pair p;
	p.x = parse1f(ss, delim, errStr);
	p.y = parse1f(ss, delim, errStr);
	return p;
}

Triple ObjFileReader::parse3f(stringstream& ss, char delim, string errStr)
{
	Triple t;
	t.x = parse1f(ss, delim, errStr);
	t.y = parse1f(ss, delim, errStr);
	t.z = parse1f(ss, delim, errStr);
	return t;
}

// == int parser ==

bool ObjFileReader::parse1ui(stringstream& ss, char delim, unsigned int& outInt, string errStr)
{
	string out = "";
	getline(ss, out, delim);
	if (out.size() == 0) 
	{ 
		outInt = 0u; 
		return false;
	}
	else
	{
		outInt = (unsigned int)stoi(out.c_str());
		return true;
	}
}

// == special parser ==

bool ObjFileReader::parseEOL(stringstream& ss, char delim)
{
	string str = "";
	streampos save = ss.tellg();
	bool isEOL = !(bool)getline(ss, str, delim);
	ss.seekg(save);
	save = ss.tellg();
	return isEOL;
}

void ObjFileReader::parseEOL(stringstream& ss, char delim, string errStr)
{
	if (!parseEOL(ss, delim)) throw invalid_argument(errStr);
}

FaceType ObjFileReader::parseSubFace(stringstream& ss, char delim, vector<unsigned int>& outVec, string errStr)
{
	FaceType newType = FaceType::V;	
	outVec.clear();

	// parse V
	unsigned int id = 0u;
	parse1ui(ss, delim, id, errStr);
	outVec.push_back(id);
	if (parseEOL(ss, delim)) return newType; // if ended then it contains only V

	// parse VT or Nothing
	if (parse1ui(ss, delim, id, errStr))
	{ 
		outVec.push_back(id);
		newType = FaceType::V_VT;
		if (parseEOL(ss, delim)) return newType;
	}

	// parse VN or Nothing
	if (parse1ui(ss, delim, id, errStr))
	{ 
		outVec.push_back(id);
		if (newType == FaceType::V) newType = FaceType::V_VN;
		else if (newType == FaceType::V_VT) newType = FaceType::V_VT_VN;
	}
	parseEOL(ss, delim, errStr);

	return newType;
}

// general methods

string ObjFileReader::errString(string msg, const char* filename, string line, int lineCount)
{
	stringstream erss;
	erss << msg << ": " << line	<< "\nFile: " << filename << "\nLine: " << lineCount << "\n";
	return erss.str();
}

// ============== load custom csv vertices and indices ====================

vector<float> readVerticesCSV(const char* filename)
{
	ifstream inputFile;
	inputFile.open(filename);

	string line = "";
	vector<float> vertices;
	while (getline(inputFile, line))
	{
		stringstream inputString(line);

		string tempString = "";
		while (getline(inputString, tempString, ','))
		{
			float value = stof(tempString.c_str());
			vertices.push_back(value);
			tempString = "";
		}

		line = "";

	}

	inputFile.close();
	return vertices;
}

vector<unsigned int> readIndicesCSV(const char* filename)
{
	ifstream inputFile;
	inputFile.open(filename);

	string line = "";
	vector<unsigned int> indices;
	while (getline(inputFile, line))
	{
		stringstream inputString(line);

		string tempString = "";
		while (getline(inputString, tempString, ','))
		{
			unsigned int value = stoi(tempString.c_str());
			indices.push_back(value);
			tempString = "";
		}

		line = "";

	}

	inputFile.close();
	return indices;
}