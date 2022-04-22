
#include "modelReader.h"

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
	LINE_INDEX,
};

enum class MtlKeywords
{
	// MAIN KEYWORDS
	NULL_KEYWORD,
	COMMENT,
	MATERIAL,
	K_AMBIENT,
	K_DIFFUSE,
	K_SPECULAR,
	N_SHININESS,
	OPACITY,
	TRANSPARENCY,
	TRANSMISSION_FILTER_COLOR,
	N_OPTICAL_DENSITY,
	ILLUMINATION_MODEL,
	MAP_AMBIENT,
	MAP_DIFFUSE,
	MAP_SPECULAR,
	MAP_ALPHA,
	MAP_BUMP_1,
	MAP_BUMP_2,
	MAP_DISPLACEMENT,
	MAP_STENCIL_DECAL,
	// TEXTURE KEYWORDS
	TEXTURE_OPTION
};

enum class FaceType
{
	NO_TYPE,
	V,
	V_VT_VN,
	V_VN,
	V_VT
};

// tokenize keywords
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
	{	LINE_INDEX,			"l"			},
};
static map< const string, ObjKeywords> objValMap = reverse_map(objKeyMap);

static map< MtlKeywords, const string > mtlKeyMap = {
	{	MtlKeywords::COMMENT,				"#"			},
	{	MtlKeywords::MATERIAL,				"newmtl"	},
	{	MtlKeywords::K_AMBIENT,				"Ka"		},
	{	MtlKeywords::K_DIFFUSE,				"Kd"		},
	{	MtlKeywords::K_SPECULAR,			"Ks"		},
	{	MtlKeywords::N_SHININESS,			"Ns"		},
	{	MtlKeywords::N_OPTICAL_DENSITY,		"Ni"		},
	{	MtlKeywords::OPACITY,				"d"			},
	{	MtlKeywords::TRANSPARENCY,			"Tr"		},
	{	MtlKeywords::MAP_DIFFUSE,			"map_Kd"	},
	{	MtlKeywords::ILLUMINATION_MODEL,	"illum"		},
};
static map< const string, MtlKeywords> mtlKeyVal = reverse_map(mtlKeyMap);



// =============== Main Functions ==================

ObjectFileData ObjFileReader::read(const char* filename, bool parseMtl)
{
	ObjectFileData data = readObj(filename);
	expandVertices(data);
	cout << "Object loaded: " << data.objFilename << endl;
	if (parseMtl) // cuz functionality of mtl loader is incomplete (default to false)
	{
		try
		{
			readMtl(data);
			cout << "Material loaded: " << data.mtlFilename << endl;
		}
		catch (const std::exception& e)
		{
			cout << "Fail loading material file" << endl;
			cout << e.what() << endl << endl;
		}
	}
	return data;
}

ObjectFileData ObjFileReader::readObj(const char* filename) 
{
	// ignore line boolean
	bool detectedLineKeyword = false;

	// input
	ifstream inputFile(filename);

	if (!inputFile.good()) throw invalid_argument("ObjFileReader::file doesn't exist");
	ObjectFileData data;
	string sFilename(filename);
	data.objFilename = sFilename;

	// intermediate data
	char delim = ' ';
	char faceDelim = '/';


	string line = "";
	FaceType faceType = FaceType::NO_TYPE;
	FaceType detectedFaceType = FaceType::NO_TYPE;
	Vector2 tempP;
	Vector3 tempT;
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
			case LINE_INDEX:
				if (!detectedLineKeyword)
				{
					cout << errString("ObjFileReader::Warning line vertex is not supported, thus ignored", 
						filename, line, lineCount);
				}
				detectedLineKeyword = true;
				break;
			case COMMENT: // do nothing
				break;
			
			case MATERIAL_FILE:
				parse1s(inputString, delim, subStr);
				data.mtlFilename = replaceBasename(data.objFilename, subStr);
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

	return data;
}

void ObjFileReader::expandVertices(ObjectFileData& data)
{
	for (int i = 0; i < data.subObjects.size(); i++)
	{
		SubObj& subObjI = data.subObjects[i];

		vector<Vector3>& ver = data.vertices;
		vector<Vector2>& tex = data.texCoords;
		vector<Vector3>& nor = data.normals;

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

void ObjFileReader::readMtl(ObjectFileData& objFd) 
{
	ifstream inputFile(objFd.mtlFilename.c_str());
	if (!inputFile.good()) throw invalid_argument("ObjFileReader::mtl file not found");

	MaterialFileData& data = objFd.mtlFileData;

	Vector3 tempV3;
	float tempF;

	char delim = ' ';
	string line = "";
	int lineCount = 0;
	while (getline(inputFile, line))
	{
		string subStr = "";
		stringstream inputLine(line);
		getline(inputLine, subStr, delim);

		if (subStr.size())
		{
			switch (mtlKeyVal[(const string)subStr])
			{
				// not supported
				case MtlKeywords::NULL_KEYWORD:
				case MtlKeywords::TEXTURE_OPTION:
				case MtlKeywords::TRANSMISSION_FILTER_COLOR:
				case MtlKeywords::MAP_BUMP_1:
				case MtlKeywords::MAP_BUMP_2:
				case MtlKeywords::MAP_DISPLACEMENT:
				case MtlKeywords::MAP_STENCIL_DECAL:
				case MtlKeywords::MAP_AMBIENT:
				case MtlKeywords::MAP_SPECULAR:
				case MtlKeywords::MAP_ALPHA:
					cout << errString("ObjFileReader::Warning unsupported material config, ignored line",
						objFd.mtlFilename.c_str(), line, lineCount);
					break;
				// ignored
				case MtlKeywords::COMMENT:
					break;
				case MtlKeywords::MATERIAL:
					parse1s(inputLine, delim, subStr);
					data.materials.push_back(SubMtl());
					data.materialNames.emplace(subStr, data.materialNames.size() + 1);
					parseEOL(inputLine, delim,
						errString("ObjFileReader::too many material names",
							objFd.mtlFilename.c_str(), line, lineCount));
					break;
				case MtlKeywords::K_AMBIENT:
					tempV3 = parse3f(inputLine, delim, subStr);
					data.materials.back().ambientColor.x = tempV3.x;
					data.materials.back().ambientColor.y = tempV3.y;
					data.materials.back().ambientColor.z = tempV3.z;
					parseEOL(inputLine, delim,
						errString("ObjFileReader::ambient color field more than 3 values",
							objFd.mtlFilename.c_str(), line, lineCount));
					break;
				case MtlKeywords::K_DIFFUSE:
					tempV3 = parse3f(inputLine, delim, subStr);
					data.materials.back().diffuseColor.x = tempV3.x;
					data.materials.back().diffuseColor.y = tempV3.y;
					data.materials.back().diffuseColor.z = tempV3.z;
					parseEOL(inputLine, delim,
						errString("ObjFileReader::diffuse color field more than 3 values",
							objFd.mtlFilename.c_str(), line, lineCount));
					break;
				case MtlKeywords::K_SPECULAR:
					tempV3 = parse3f(inputLine, delim, subStr);
					data.materials.back().specularColor.x = tempV3.x;
					data.materials.back().specularColor.y = tempV3.y;
					data.materials.back().specularColor.z = tempV3.z;
					parseEOL(inputLine, delim,
						errString("ObjFileReader::specular color field more than 3 values",
							objFd.mtlFilename.c_str(), line, lineCount));
					break;
				case MtlKeywords::N_SHININESS:
					tempF = parse1f(inputLine, delim, subStr);
					data.materials.back().shininess = tempF;
					parseEOL(inputLine, delim,
						errString("ObjFileReader::shiniess field more than 1 value",
							objFd.mtlFilename.c_str(), line, lineCount));
					break;
				case MtlKeywords::OPACITY:
					tempF = parse1f(inputLine, delim, subStr);
					data.materials.back().opacity = tempF;
					parseEOL(inputLine, delim,
						errString("ObjFileReader::opacity field more than 1 value",
							objFd.mtlFilename.c_str(), line, lineCount));
					break;
				case MtlKeywords::N_OPTICAL_DENSITY:
					tempF = parse1f(inputLine, delim, subStr);
					data.materials.back().opticalDensity = tempF;
					parseEOL(inputLine, delim,
						errString("ObjFileReader::optical density field more than 1 value",
							objFd.mtlFilename.c_str(), line, lineCount));
					break;
				case MtlKeywords::TRANSPARENCY:
					tempF = parse1f(inputLine, delim, subStr);
					data.materials.back().opacity = 1 - tempF;
					parseEOL(inputLine, delim,
						errString("ObjFileReader::transparency field more than 1 value",
							objFd.mtlFilename.c_str(), line, lineCount));
					break;
				case MtlKeywords::ILLUMINATION_MODEL:
					tempF = parse1f(inputLine, delim, subStr);
					data.materials.back().illuminationModel = (int) tempF;
					parseEOL(inputLine, delim,
						errString("ObjFileReader::illumination model field more than 1 value",
							objFd.mtlFilename.c_str(), line, lineCount));
					break;
				case MtlKeywords::MAP_DIFFUSE:
					parse1s(inputLine, delim, subStr);
					parseEOL(inputLine, delim,
						errString("ObjFileReader::texture map option is not supported",
							objFd.mtlFilename.c_str(), line, lineCount));
					if (!data.textureFilenames[subStr])
					{
						int idx = data.textureFilenames.size();
						data.textureFilenames.emplace(subStr, idx + 1);
						data.materials.back().diffuseColorTextureIdx = idx;
					}
					else
					{
						data.materials.back().diffuseColorTextureIdx = data.textureFilenames[subStr] - 1;
					}
					break;
			}

		}

		line = "";
		lineCount++;
	}

	inputFile.close();
}


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

Vector2 ObjFileReader::parse2f(stringstream& ss, char delim, string errStr) 
{
	Vector2 p;
	p.x = parse1f(ss, delim, errStr);
	p.y = parse1f(ss, delim, errStr);
	return p;
}

Vector3 ObjFileReader::parse3f(stringstream& ss, char delim, string errStr)
{
	Vector3 t;
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

bool ObjFileReader::checkEOL(stringstream& ss, char delim)
{
	string str = "";
	streampos save = ss.tellg();
	bool isEOL = !(bool)getline(ss, str, delim);
	ss.seekg(save);
	return isEOL;
}

//bool ObjFileReader::parseIfMatch(stringstream& ss, char delim, string compared)
//{
//	string str = "";
//	streampos save = ss.tellg();
//	getline(ss, str, delim);
//	if (!str.compare(compared))
//	{
//		ss.seekg(save);
//		return false;
//	}
//	return true;
//}

void ObjFileReader::parseEOL(stringstream& ss, char delim, string errStr)
{
	if (!checkEOL(ss, delim)) throw invalid_argument(errStr);
}

FaceType ObjFileReader::parseSubFace(stringstream& ss, char delim, vector<unsigned int>& outVec, string errStr)
{
	FaceType newType = FaceType::V;	
	outVec.clear();

	// parse V
	unsigned int id = 0u;
	parse1ui(ss, delim, id, errStr);
	outVec.push_back(id);
	if (checkEOL(ss, delim)) return newType; // if ended then it contains only V

	// parse VT or Nothing
	if (parse1ui(ss, delim, id, errStr))
	{ 
		outVec.push_back(id);
		newType = FaceType::V_VT;
		if (checkEOL(ss, delim)) return newType;
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

string ObjFileReader::replaceBasename(string filename, string basename)
{
	string filenameBase = filename.substr((filename.find_last_of("/\\") + 1));
	string root = filename.substr(0, filename.size() - filenameBase.size());
	return root + basename;
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