
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
	V_VT_VN = 1,
	V_VN = 2,
	V_VT = 3
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
	char sub_delim = '/';

	WholeObj data;
	string line = "";
	FaceType faceType = FaceType::NO_TYPE;
	vector<float> tempf;
	vector<unsigned int> tempui;
	
	int lineCount = 0;
	int objectCount = 0;

	while (getline(inputFile, line))
	{		
		stringstream inputString(line);

		// first element of the line

		string subStr = "";
		getline(inputString, subStr, delim);
		ObjKeywords key = objValMap[(const string)subStr];
		
		switch (key)
		{
		case NULL_KEYWORD:
			throw invalid_argument(
				errString("ObjFileReader::Unhandle Keyword line", filename, 
					line, lineCount)
			);
			break;

		case COMMENT: // do nothing
			break;
			
		case MATERIAL_FILE:
			parse1s(inputString, subStr, delim);		
			data.mtl_file = subStr;		
			parseEOL(
				inputString, 
				errString("ObjFileReader::Too many paths for a single material file", filename, 
					line, lineCount), 
				delim
			);
			break;

		case OBJECT:
			data.rawObjects.push_back(RawObj());		
			parse1s(inputString, subStr, delim);		
			data.rawObjects.back().modelObjectName = subStr;
			parseEOL(
				inputString, 
				errString("ObjFileReader::Too many names for a single object", filename, 
					line, lineCount), 
				delim
			);
			faceType = FaceType::NO_TYPE; // for parsing face later on
			break;

		case GROUP:
			throw invalid_argument(
				errString("ObjFileReader::Object grouping is not handled", filename, 
					line, lineCount)
			);
			break;

		case VERTEX:			
			tempf = parseNf(
				inputString, 3, delim, 
				errString("ObjFileReader::Length of vertex coordinate is not 3", filename, 
					line, lineCount)
			);
			data.rawObjects.back().vertices.insert(
				end(data.rawObjects.back().vertices),
				begin(tempf),
				end(tempf)
			);
			parseEOL(
				inputString, 
				errString("ObjFileReader::More than 3 points in a vertex, vertex must be trangulated", 
					filename, line, lineCount), 
				delim
			);
			break;

		case TEXTURE_MAP:			
			tempf = parseNf(
				inputString, 2, delim, 
				errString("ObjFileReader::Length of texture coordinate is not 2",
					filename, line, lineCount)
			);
			data.rawObjects.back().textureMap.insert(
				end(data.rawObjects.back().textureMap),
				begin(tempf),
				end(tempf)
			);
			parseEOL(
				inputString, 
				errString("ObjFileReader::More than 3 points in a vertex", 
					filename, line, lineCount), 
				delim
			);
			break;

		case NORMAL:			
			tempf = parseNf(
				inputString, 3, delim, 
				errString("ObjFileReader::Length of normals is not 3",
				filename, line, lineCount)
			);
			data.rawObjects.back().normals.insert(
				end(data.rawObjects.back().normals),
				begin(tempf),
				end(tempf)
			);
			parseEOL(
				inputString,
				errString("ObjFileReader::More than 3 values in a normals",
					filename, line, lineCount),
				delim
			);
			break;

		case USE_MATERIAL:
			parse1s(inputString, subStr, delim);
			data.rawObjects.back().useMaterial = subStr;			
			parseEOL(
				inputString, 
				errString("ObjFileReader::Use too many materials",
					filename, line, lineCount), 
				delim
			);
			break;

		case SMOOTH_SHADDING:
			parse1s(inputString, subStr, delim);
			data.rawObjects.back().smoothShadding = subStr;
			parseEOL(
				inputString, 
				errString("ObjFileReader::Too many arguments in smooth shadding",
					filename, line, lineCount), 
				delim
			);
			break;

		case FACE_INDEX:			
			for (int i = 0; i < 3; i++)
			{
				parse1s(inputString, subStr, delim);
				stringstream idStream(subStr);
				tempui = parseFace(
					idStream, faceType, sub_delim,
					errString("ObjFileReader::Invalid Face Indices",
						filename, line, lineCount)
				);
				switch (faceType)
				{
				case FaceType::V_VT_VN:
					data.rawObjects.back().verticesIdx.push_back(tempui[0]);
					data.rawObjects.back().textureMapIdx.push_back(tempui[1]);
					data.rawObjects.back().normalsIdx.push_back(tempui[2]);
					break;
				case FaceType::V_VN:
					data.rawObjects.back().verticesIdx.push_back(tempui[0]);
					data.rawObjects.back().normalsIdx.push_back(tempui[1]);
					break;
				case FaceType::V_VT:
					data.rawObjects.back().verticesIdx.push_back(tempui[0]);
					data.rawObjects.back().textureMapIdx.push_back(tempui[1]);
					break;
				}
			}
			parseEOL(
				inputString,
				errString("ObjFileReader::More than 3 sets of indices, please triangulate object",
					filename, line, lineCount),
				delim
			);
			break;
		}

		// end of line reset line string
		tempf.clear();
		tempui.clear();
		line = "";
		lineCount++;
	}
	inputFile.close();

	expandVertices(data);

	return data;
}



void ObjFileReader::expandVertices(WholeObj& data)
{
	// TODO FIX ACCUMULATIVE INDEX (to allow indexing from other sub objects)

	for (int i = 0; i < data.rawObjects.size(); i++)
	{
		RawObj& rawObjI = data.rawObjects[i];
		bool texIsNotEmpty = (rawObjI.textureMap.size() != 0);
		bool norIsNotEmpty = (rawObjI.normals.size() != 0);

		for (int j = 0; j < data.rawObjects[i].verticesIdx.size(); j++)
		{

			int vIdx = (rawObjI.verticesIdx[j] - 1) * 3;
			rawObjI.expandedVertices.push_back(rawObjI.vertices[vIdx    ]); // vx
			rawObjI.expandedVertices.push_back(rawObjI.vertices[vIdx + 1]); // vy
			rawObjI.expandedVertices.push_back(rawObjI.vertices[vIdx + 2]); // vz

			int tIdx = (rawObjI.textureMapIdx[j] - 1) * 2;
			if (texIsNotEmpty) 
			{
				rawObjI.expandedVertices.push_back(rawObjI.textureMap[tIdx]);	  // u
				rawObjI.expandedVertices.push_back(rawObjI.textureMap[tIdx + 1]); // v
			}

			int nIdx = (rawObjI.normalsIdx[j] - 1) * 3;
			if (norIsNotEmpty)
			{
				rawObjI.expandedVertices.push_back(rawObjI.normals[nIdx]);	   // nx
				rawObjI.expandedVertices.push_back(rawObjI.normals[nIdx + 1]); // ny
				rawObjI.expandedVertices.push_back(rawObjI.normals[nIdx + 2]); // nz
			}

			cout << i << " " << j << " " << vIdx << " " << tIdx << " " << nIdx << endl;
		}
		
	}

};



// ========== Auxilliary Functions =============

bool ObjFileReader::parse1s(stringstream& ss, string& outputString, char delim)
{
	return (bool)getline(ss, outputString, delim);
}

void ObjFileReader::parseEOL(stringstream& ss, string errStr, char delim)
{
	string str = "";
	if (parse1s(ss, str, delim)) throw invalid_argument(errStr);
	return;
}

float ObjFileReader::parse1f(stringstream& ss, string errStr, char delim)
{
	string out = "";
	getline(ss, out, delim);
	return stof(out.c_str());
}

unsigned int ObjFileReader::parse1ui(stringstream& ss, string errStr, char delim)
{
	string out = "";
	getline(ss, out, delim);
	if (out.size() == 0) return NULL; // based on the fact that index in obj files are not starting from 0
	return (unsigned int) stoi(out.c_str());
}

vector<float> ObjFileReader::parseNf(stringstream& ss, unsigned int n, char delim, string errStr)
{
	vector<float> point;
	for (int i = 0; i < n; i++)
	{
		point.push_back(parse1f(ss, errStr, delim));
	}
	if (point.size() != n) throw invalid_argument(errStr);
	return point;
}

vector<unsigned int> ObjFileReader::parseFace(stringstream& ss, FaceType& type, char delim, string errStr)
{
	vector<unsigned int> idxs;
	FaceType newType = FaceType::NO_TYPE;

	// parse V
	unsigned int id = parse1ui(ss, errStr, delim);
	idxs.push_back(id);

	// parse VT or Nothing
	id = parse1ui(ss, errStr, delim);
	if (id)
	{
		idxs.push_back(id);
		newType = FaceType::V_VT;
	} 
	else
	{
		newType = FaceType::V_VN;
	}

	// parse VN or Nothing
	id = parse1ui(ss, errStr, delim);
	if (id)
	{
		idxs.push_back(id);
		if (newType == FaceType::V_VT)	newType = FaceType::V_VT_VN;
	}
	else
	{
		throw invalid_argument(errStr);
	}
	
	if (type == FaceType::NO_TYPE)
	{
		type = newType;
	} 
	else if (type != newType)
	{
		throw invalid_argument(errStr);
	}

	return idxs;
}

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