
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <stdlib.h>
#include "stb_image.h"
#include "shader.h"
#include "window.h"
#include "modelReader.h"
#include "shapes.h"
#include "OrbitAnimator.h"
#include "SceneState.h"
#include "PlanetMath.h"
#include "GeneralCamera.h"

// debug
//#include <glm/glm/gtx/string_cast.hpp>

using namespace std;

// ======================= prototype =======================

// IO function
void processKeyboard(GLFWwindow* window);
void processMouse(GLFWwindow* window, double x, double y);

// animation keyboard interaction
void updateAnimatorsDelays();
void randomizeOrbitAngles();

// load function
unsigned int loadCubemap(vector<string> filename);
unsigned int loadTexture(const char* filename);

// opengl helper
void glSetupVertexObject(unsigned int& VAO, unsigned int& VBO, vector<float>& data, vector<int> attribLayout);
void glDrawVertexTriangles(unsigned int VAO, GLuint texture, int numberOfVertex);
void glSetModelViewProjection(unsigned int shaderProgram, glm::mat4 model, glm::mat4 view, glm::mat4 projection);
void glSetLightingConfig(unsigned int shaderProgram, glm::vec3 lightPos, GeneralCamera camPos, int torch);

// helper
glm::vec3 vecToVec3(vector<float> vec);
vector<float> vec3ToVec(glm::vec3 vec3);

// opengl code dump
void displayLoadingScreen(GLFWwindow* window);
void displaySkyBox(unsigned int& VAO, GLuint texture, unsigned int shaderProgram, glm::mat4 view, glm::mat4 projection);


// ====================== global variable =======================

// settings
int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 800;
float targetFPS = 144.f;
DelayTrigger frameTrigger = DelayTrigger(1.f / targetFPS);

// camera and camera control
GeneralCamera camera;
bool firstMouse = true;
float prevMouseX;
float prevMouseY;

// scene 
int sunIdx = 0;		// THIS MUST BE CHANGED WHENEVER THE CONFIGURATION MATRIX IS CHANGED
int earthIdx = 4;   // THIS MUST BE CHANGED WHENEVER THE CONFIGURATION MATRIX IS CHANGED
float earthOrbitDelay = 3600;
glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 0.0f);
PlanetMath m;
vector<RenderedBody> renderedBodies;
vector<BodyConst> bodyConstants;
vector<OrbitAnimator> animators;

// camera mode 
DelayTrigger fTrigger;
DelayTrigger rTrigger;
DelayTrigger leftCtrlTrigger;
DelayTrigger qTrigger;
DelayTrigger eTrigger;
bool cameraMode = false;
int modelSelection = 0;

// camera movement
DelayTrigger wTrigger = DelayTrigger(0.002f);
DelayTrigger aTrigger = DelayTrigger(0.002f);
DelayTrigger sTrigger = DelayTrigger(0.002f);
DelayTrigger dTrigger = DelayTrigger(0.002f);
DelayTrigger leftShiftTrigger = DelayTrigger(0.002f);
DelayTrigger spaceTrigger = DelayTrigger(0.002f);
DelayTrigger zTrigger = DelayTrigger(0.002f);
DelayTrigger cTrigger = DelayTrigger(0.002f);

// scene adjustment
SceneState sceneState;
DelayTrigger commaTrigger = DelayTrigger(0.002f);
DelayTrigger periodTrigger = DelayTrigger(0.002f);
DelayTrigger leftBracketTrigger = DelayTrigger(0.002f);
DelayTrigger rightBracketTrigger = DelayTrigger(0.002f);


// ==================== main =======================

int main(int argc, char** argv)
{

	// ======================= SETUP ======================	

	srand((int)time(NULL));
	GLFWwindow* window = myCreateWindow(
		WINDOW_WIDTH, WINDOW_HEIGHT, "Space Scene");	// create window
	mySetWindowCenter(window);							// adjust window position
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // disable mouse
	glfwSetCursorPosCallback(window, processMouse);		// set mouse event callback
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); // init glad
	

	// ====================== OPENGL ======================		

	displayLoadingScreen(window);	// loading screen (contains gl code)
	int startLoadingTime = (int)glfwGetTime(); // to calculate loading time


	// ========= load objects =========

	ObjFileReader ofr;
	ObjectFileData sphereObj, ufoObj, rocket2Obj, saturnRingObj, uranusRingObj, astroid1Obj, 
		commandModuleObj, electronRocketObj, satelite1Obj, superHeavyRocketObj;
	cout << "Loading Objects...\n";
	try
	{
		sphereObj = ofr.read("resources/solar_system/sphere.obj");
		ufoObj = ofr.read("resources/ufo_1/ufo_1.obj");
		rocket2Obj = ofr.read("resources/rocket_2/rocket_2.obj");
		saturnRingObj = ofr.read("resources/solar_system/ring_huge.obj");
		uranusRingObj = ofr.read("resources/solar_system/ring_small.obj");
		astroid1Obj = ofr.read("resources/astroid_1/astroid_1.obj");
		commandModuleObj = ofr.read("resources/command_module/command_module.obj");
		electronRocketObj = ofr.read("resources/electron/electron.obj");
		satelite1Obj = ofr.read("resources/satelite_1/satelite_1.obj");
		superHeavyRocketObj = ofr.read("resources/super_heavy/super_heavy.obj");
	}
	catch (const std::exception& e)
	{
		cerr << "Fail to load object file\n";
		cerr << e.what() << endl;
		return -1;
	}
	vector<float> skyboxVert = getSkyboxCube();
	vector<float>& sphereVert = sphereObj.subObjects[0].expandedVertices;
	vector<float>& ufoVert = ufoObj.subObjects[0].expandedVertices;
	vector<float>& rocket2Vert = rocket2Obj.subObjects[0].expandedVertices;
	vector<float>& saturnRingVert = saturnRingObj.subObjects[0].expandedVertices;
	vector<float>& uranusRingVert = uranusRingObj.subObjects[0].expandedVertices;
	vector<float>& astroid1Vert = astroid1Obj.subObjects[0].expandedVertices;
	vector<float>& commandModuleVert = commandModuleObj.subObjects[0].expandedVertices;
	vector<float>& electronRocketVert = electronRocketObj.subObjects[0].expandedVertices;
	vector<float>& satelite1Vert = satelite1Obj.subObjects[0].expandedVertices;
	vector<float>& superHeavyRocketVert = superHeavyRocketObj.subObjects[0].expandedVertices;
	cout << "Objects Loaded\n\n";


	// ======== load shaders =========

	cout << "Loading Shaders...\n";
	unsigned int illumShaderProgram = LoadShader("shaders/illuminated.vert", "shaders/illuminated.frag");
	unsigned int earthShaderProgram = LoadShader("shaders/earth.vert", "shaders/earth.frag");
	unsigned int basicShaderProgram = LoadShader("shaders/basic.vert", "shaders/basic.frag");
	unsigned int skyShaderProgram = LoadShader("shaders/sky.vert", "shaders/sky.frag");
	cout << "Shaders Loaded\n\n";

	vector<unsigned int> shaders{
		basicShaderProgram,
		illumShaderProgram,
		earthShaderProgram,
	};


	// ======= load all textures =======

	cout << "Loading Textures...\n";
	GLuint sunTexture = loadTexture("resources/solar_system/textures/2k_sun.jpg");
	GLuint mercuryTexture = loadTexture("resources/solar_system/textures/2k_mercury.jpg");
	GLuint venusTexture = loadTexture("resources/solar_system/textures/2k_venus_surface.jpg");
	//GLuint venusAtmosphereTexture = loadTexture("resources/solar_system/textures/2k_venus_atmosphere.jpg");
	GLuint earthTexture = loadTexture("resources/solar_system/textures/2k_earth_daymap.jpg");
	GLuint earthNightTexture = loadTexture("resources/solar_system/textures/8k_earth_nightmap.jpg");
	GLuint earthCloudsTexture = loadTexture("resources/solar_system/textures/2k_earth_clouds.jpg");
	GLuint moonTexture = loadTexture("resources/solar_system/textures/2k_moon.jpg");
	GLuint marsTexture = loadTexture("resources/solar_system/textures/2k_mars.jpg");
	GLuint jupiterTexture = loadTexture("resources/solar_system/textures/2k_jupiter.jpg");
	GLuint saturnTexture = loadTexture("resources/solar_system/textures/2k_saturn.jpg");
	GLuint saturnRingTexture = loadTexture("resources/solar_system/textures/saturn_ring_2.png");
	GLuint uranusTexture = loadTexture("resources/solar_system/textures/2k_uranus.jpg");
	GLuint uranusRingTexture = loadTexture("resources/solar_system/textures/uranus_ring_2.png");
	GLuint neptuneTexture = loadTexture("resources/solar_system/textures/2k_neptune.jpg");
	GLuint plutoTexture = loadTexture("resources/solar_system/textures/pluto.jpg");
	GLuint ufoTexture = loadTexture("resources/ufo_1/ufo_kd.jpg");
	GLuint rocket2Texture = loadTexture("resources/rocket_2/rocket.jpg");
	GLuint astroid1Texture = loadTexture("resources/astroid_1/astroid_1.jpg");
	GLuint commandModuleTexture = loadTexture("resources/command_module/command_module.png");
	GLuint electronRocketTexture = loadTexture("resources/electron/electron.png");
	GLuint satelite1Texture = loadTexture("resources/satelite_1/satelite_1.jpg");
	GLuint superHeavyRocketTexture = loadTexture("resources/super_heavy/super_heavy.png");
	vector<string> files = {
		"resources/milkyway/right.png",
		"resources/milkyway/left.png",
		"resources/milkyway/bottom.png",
		"resources/milkyway/top.png",
		"resources/milkyway/front.png",
		"resources/milkyway/back.png"
	};
	GLuint skyTexture = loadCubemap(files);
	cout << "Textures Loaded\n\n";

	vector<vector<GLuint>> textures{
		{ sunTexture },
		{ mercuryTexture },
		{ venusTexture },
		{ earthTexture, earthCloudsTexture, earthNightTexture},
		{ marsTexture },
		{ jupiterTexture },
		{ saturnTexture },
		{ uranusTexture },
		{ neptuneTexture },
		{ plutoTexture },
		{ moonTexture },
		{ ufoTexture },
		{ rocket2Texture },
		{ saturnRingTexture },
		{ uranusRingTexture },
		{ astroid1Texture },
		{ commandModuleTexture },
		{ electronRocketTexture },
		{ satelite1Texture },
		{ superHeavyRocketTexture },
	};


	// ======= prepre scene rendering =======

	cout << "Setting Up Scene...\n";
	// gen buffers
	unsigned int sphereVAO, sphereVBO;
	glSetupVertexObject(sphereVAO, sphereVBO, sphereVert, vector<int>{3, 2, 3});
	unsigned int ufoVAO, ufoVBO;
	glSetupVertexObject(ufoVAO, ufoVBO, ufoVert, vector<int>{3, 2, 3});
	unsigned int rocket2VAO, rocket2VBO;
	glSetupVertexObject(rocket2VAO, rocket2VBO, rocket2Vert, vector<int>{3,2,3});
	unsigned int saturnRingVAO, saturnRingVBO;
	glSetupVertexObject(saturnRingVAO, saturnRingVBO, saturnRingVert, vector<int>{3, 2, 3});
	unsigned int uranusRingVAO, uranusRingVBO;
	glSetupVertexObject(uranusRingVAO, uranusRingVBO, uranusRingVert, vector<int>{3, 2, 3});
	unsigned int astroid1VAO, astroid1VBO;
	glSetupVertexObject(astroid1VAO, astroid1VBO, astroid1Vert, vector<int>{3, 2, 3});
	unsigned int commandModuleVAO, commandModuleVBO;
	glSetupVertexObject(commandModuleVAO, commandModuleVBO, commandModuleVert, vector<int>{3, 2, 3});
	unsigned int electronRocketVAO, electronRocketVBO;
	glSetupVertexObject(electronRocketVAO, electronRocketVBO, electronRocketVert, vector<int>{3, 2, 3});
	unsigned int satelite1VAO, satelite1VBO;
	glSetupVertexObject(satelite1VAO, satelite1VBO, satelite1Vert, vector<int>{3, 2, 3});
	unsigned int superHeavyRocketVAO, superHeavyRocketVBO;
	glSetupVertexObject(superHeavyRocketVAO, superHeavyRocketVBO, superHeavyRocketVert, vector<int>{3, 2, 3});
	unsigned int skyVAO, skyVBO;
	glSetupVertexObject(skyVAO, skyVBO, skyboxVert, vector<int>{3});

	vector<unsigned int> VAOs{
		sphereVAO,
		ufoVAO,
		rocket2VAO,
		saturnRingVAO,
		uranusRingVAO,
		astroid1VAO,
		commandModuleVAO,
		electronRocketVAO,
		satelite1VAO,
		superHeavyRocketVAO,
	};

	vector<int> vertexSize{
		(int)sphereVert.size() / 8,
		(int)ufoVert.size() / 8,
		(int)rocket2Vert.size() / 8,
		(int)saturnRingVert.size() / 8,
		(int)uranusRingVert.size() / 8,
		(int)astroid1Vert.size() / 8,
		(int)commandModuleVert.size() / 8,
		(int)electronRocketVert.size() / 8,
		(int)satelite1Vert.size() / 8,
		(int)superHeavyRocketVert.size() / 8,
	};

	// remove binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	// =========== MODEL & ANIMATION CONFIG ==============


	// model constants
	float SPHERE_OBJECT_RADIUS = 2;		// 3d vertex sphere radius (do not change)

	// model hyper params				(tweak these to adjust scene)

	float distanceModifier = 240;		// master distance margin scale
	float earthScale = 200;				// master scale

	// all these values have to change if customization structure is changed
	int attributeCount = 10;
	vector<float> bodiesCustomization{

		// esthetic
		// a. is animated boolean 
		// s. scale
		// m. margin, the distance between the previous orbiting planet and 
		//		current planet
		// ov. oval ratio, major minor axis ratio
		// rd. randomize spin angle boolean (this is the switch to enable fixed spin 
		//		angle initialization relative to orbit angle, as some object have to be 
		//		parpendicular to other object to make sense)
		// 
		// resource linking
		// pr. parent index (-1: not orbiting, * > -1: orbiting pr when a == 1, 
		//		following pr when a == 0) [refer to this array]
		// bc. body constant index [refer to "bodyConstants" array]
		// vao. VAO and vertex size index [refer to "vertexSize" & "VAOs" array]
		// tx. texture index [refer to "textures" array]
		// mv. model view boolean to disable model view option of certain objects
		//
		// rules: orbited object must come before orbiting object as some calculations 
		//			are depending on their primary object
		// earthIdx MUST BE CHANGED WHENEVER THE CONFIGURATION MATRIX IS CHANGED
		// sunIdx MUST BE CHANGED WHENEVER THE CONFIGURATION MATRIX IS CHANGED
		// 
		//  a		s		m		ov		rd		pr*		bc		vao		tx		mv
			0.f,	0.3f,	0.f,	1.f,	1.f,	-1.f,	0.f,	0.f,	0.f,	1.f, // 0. sun
			1.f,	1.f,	20.f,	1.f,	1.f,	0.f,	1.f,	0.f,	1.f,	1.f, // 1. mercury
			1.f,	1.f,	20.f,	1.f,	1.f,	0.f,	2.f,	0.f,	2.f,	1.f, // 2. venus
			1.f,	0.6f,	5.f,	1.2f,	0.f,	2.f,	19.f,	7.f,	17.f,	1.f, // 3. electron rocket
			1.f,	1.f,	20.f,	1.f,	1.f,	0.f,	3.f,	0.f,	3.f,	1.f, // 4. earth
			1.f,	0.6f,	3.f,	1.2f,	1.f,	4.f,	17.f,	6.f,	16.f,	1.f, // 5. apollo 11 command module
			1.f,	0.2f,	2.f,	1.f,	1.f,	4.f,	20.f,	8.f,	18.f,	1.f, // 5. satelite 1
			1.f,	1.f,	5.f,	1.f,	0.f,	4.f,	10.f,	0.f,	10.f,	1.f, // 6. moon
			1.f,	0.5f,	2.f,	1.f,	1.f,	7.f,	11.f,	1.f,	11.f,	1.f, // 7. ufo 
			1.f,	0.5f,	0.3f,	1.05f,	1.f,	7.f,	12.f,	1.f,	11.f,	1.f, // 8. ufo 
			1.f,	0.5f,	0.5f,	1.f,	1.f,	8.f,	12.f,	1.f,	11.f,	1.f, // 9. ufo 
			1.f,	1.f,	20.f,	1.f,	1.f,	0.f,	4.f,	0.f,	4.f,	1.f, // 10. mars
			1.f,	0.5f,	3.f,	1.f,	0.f,	11.f,	13.f,	2.f,	12.f,	1.f, // 11. rocket 2
			1.f,	1.f,	5.f,	1.f,	1.f,	0.f,	16.f,	5.f,	15.f,	1.f, // 12. astroid 1
			1.f,	1.f,	55.f,	1.f,	1.f,	0.f,	5.f,	0.f,	5.f,	1.f, // 13. jupiter
			1.f,	1.f,	60.f,	1.f,	1.f,	0.f,	6.f,	0.f,	6.f,	1.f, // 14. saturn
			0.f,	0.9,	0.f,	1.f,	1.f,	15.f,	14.f,	3.f,	13.f,	0.f, // 15. saturn ring
			1.f,	1.f,	20.f,	1.5f,	0.f,	15.f,	18.f,	9.f,	19.f,	1.f, // 16. super heavy rocket
			1.f,	1.f,	100.f,	1.f,	1.f,	0.f,	7.f,	0.f,	7.f,	1.f, // 17. uranus
			0.f,	0.9,	0.f,	1.f,	1.f,	18.f,	15.f,	4.f,	14.f,	0.f, // 18. uranus ring
			1.f,	1.f,	80.f,	1.f,	1.f,	0.f,	8.f,	0.f,	8.f,	1.f, // 19. neptune
			1.f,	1.f,	40.f,	1.f,	1.f,	0.f,	9.f,	0.f,	9.f,	1.f, // 20. pluto
	};
	// TODO: add as much model as possible
	// TODO: earth use multi textures, night city lights

	renderedBodies.resize(bodiesCustomization.size() / attributeCount);
	renderedBodies[sunIdx].position = vec3ToVec(lightPos);

	// get predefined body constants for solar system
	bodyConstants = m.getSolarSystemConstants();

	// add additional custom made body constants for additional objects
	// this part is just pure creativity nothing technical
	BodyConst customBc;

	// 11 ufo
	customBc.ascendingNode = 0;									// adjust orbit ascending starting point (degree)
	customBc.axialTilt = 180;									// adjust axial tilt relative to orbit plane (degree)
	customBc.inclination = 180 + 30;							// adjust orbit inclination angle (degree)
	customBc.localOrbitalPeriod = 30;							// number of spins per orbit
	customBc.orbitalPeriod = PConst::MOON_ORBITAL_PERIOD / 8;	// orbit duration
	customBc.radius = 400;										// object size (will be scaled relative to earth)
	bodyConstants.push_back(customBc);

	// 12 ufo
	customBc.axialTilt += 10;
	customBc.inclination += 10;
	customBc.ascendingNode = 90;
	customBc.orbitalPeriod = PConst::MOON_ORBITAL_PERIOD / 10;
	bodyConstants.push_back(customBc);

	// 13 rocket 2
	customBc.ascendingNode = 0;	
	customBc.axialTilt = 0;		
	customBc.inclination = 20;	
	customBc.localOrbitalPeriod = 1;						
	customBc.orbitalPeriod = 30;
	customBc.radius = 50;	
	customBc.defaultSpinAngle = 90;	 // spin angle relative to orbit angle (make rocket perpendicular to orbit center)
	bodyConstants.push_back(customBc);

	// 14 ring
	customBc.ascendingNode = 0;
	customBc.axialTilt = 2;
	customBc.inclination = 0;
	customBc.radius = PConst::SATURN_RADIUS;
	bodyConstants.push_back(customBc);

	// 15 ring
	customBc.ascendingNode = 0;
	customBc.axialTilt = PConst::URANUS_AXIAL_TILT;
	customBc.inclination = 0;
	customBc.radius = PConst::URANUS_RADIUS;
	customBc.defaultSpinAngle = 0;
	bodyConstants.push_back(customBc);

	// 16 astroid
	customBc.ascendingNode = 10;
	customBc.axialTilt = 10;
	customBc.inclination = 5;
	customBc.localOrbitalPeriod = 1;
	customBc.orbitalPeriod = 600;
	customBc.radius = PConst::MARS_RADIUS / 2;
	bodyConstants.push_back(customBc);

	// 17 command module
	customBc.ascendingNode = 40;
	customBc.axialTilt = 70;
	customBc.inclination = 40;
	customBc.localOrbitalPeriod = 0.1;
	customBc.orbitalPeriod = 30;
	customBc.radius = 500;
	bodyConstants.push_back(customBc);

	// 18 super heavy rocket
	customBc.ascendingNode = 70;
	customBc.axialTilt = 0;
	customBc.inclination = 35;
	customBc.localOrbitalPeriod = 1;
	customBc.orbitalPeriod = 60;
	customBc.radius = 500;
	customBc.defaultSpinAngle = 90;
	bodyConstants.push_back(customBc);

	// 19 electron
	customBc.ascendingNode = 20;
	customBc.axialTilt = 10;
	customBc.inclination = 35;
	customBc.localOrbitalPeriod = 1;
	customBc.orbitalPeriod = 30;
	customBc.radius = 500;
	customBc.defaultSpinAngle = 90;
	bodyConstants.push_back(customBc);

	// 20 satelite 1
	customBc.ascendingNode = 0;
	customBc.axialTilt = 20;
	customBc.inclination = 70;
	customBc.localOrbitalPeriod = 0.0001;
	customBc.orbitalPeriod = 30;
	customBc.radius = 500;
	customBc.defaultSpinAngle = 90;
	bodyConstants.push_back(customBc);

	// ========== math section ============

	// parse configurations
	int animIndexCount = 0;
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if ((bool)bodiesCustomization[i * attributeCount])
		{
			renderedBodies[i].animatorIdx = animIndexCount;
			animIndexCount++;
		}
		else
		{
			renderedBodies[i].animatorIdx = -1;
		}
		renderedBodies[i].scaleModifier = bodiesCustomization[i * attributeCount + 1];
		renderedBodies[i].distanceMargin = bodiesCustomization[i * attributeCount + 2];
		renderedBodies[i].ovalRatio = bodiesCustomization[i * attributeCount + 3];
		renderedBodies[i].randomSpinAngle = (bool)bodiesCustomization[i * attributeCount + 4];
		renderedBodies[i].orbitParentIdx = bodiesCustomization[i * attributeCount + 5];
		renderedBodies[i].bodyConstantIdx = bodiesCustomization[i * attributeCount + 6];
		renderedBodies[i].VAOIdx = bodiesCustomization[i * attributeCount + 7];
		renderedBodies[i].textureIdx = bodiesCustomization[i * attributeCount + 8];
		renderedBodies[i].modelViewed = (bool)bodiesCustomization[i * attributeCount + 9];
	}

	// compute scale relative to earth
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (i == earthIdx) continue;
		renderedBodies[i].scale = m.getRelativeValue(bodyConstants[renderedBodies[i].bodyConstantIdx].radius,
			PConst::EARTH_RADIUS, earthScale, renderedBodies[i].scaleModifier);
	}
	renderedBodies[earthIdx].scale = earthScale; // set earth scale

	// compute sphere radius for orbit radius calculation
	vector<float> sphereRadius;
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		sphereRadius.push_back(renderedBodies[i].scale * SPHERE_OBJECT_RADIUS);
	}

	// calculate orbit radius distance for individual bodies
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		// TODO following object
		if (renderedBodies[i].orbitParentIdx == -1) continue;

		// find last previous orbiting body that has the same parent 
		// if doesn't exist use current body parent index (next section)
		int preIdx = -1;
		for (int j = i - 1; j > -1; j--)
		{
			if (renderedBodies[i].orbitParentIdx == renderedBodies[j].orbitParentIdx)
			{
				preIdx = j;
				break;
			}
		}

		// calculate orbit radius, if this body is the first object that orbits it's parent then do this
		if (preIdx == -1)
		{
			renderedBodies[i].orbitRadius =
				sphereRadius[renderedBodies[i].orbitParentIdx] + sphereRadius[i] +
				(renderedBodies[i].distanceMargin * distanceModifier);
		}
		// else use previous neighbour's values to calculate orbit radius
		else
		{
			renderedBodies[i].orbitRadius =
				renderedBodies[preIdx].orbitRadius + sphereRadius[preIdx] + sphereRadius[i] +
				(renderedBodies[i].distanceMargin * distanceModifier);
		}
	}

	// calculate delays relative to earth orbiting period
	vector<float> delays;
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (renderedBodies[i].animatorIdx == -1) continue;
		if (i == earthIdx) // use delay specified by user
		{
			delays.push_back(earthOrbitDelay);
		}
		else // use delay relative to earth delay
		{
			delays.push_back(m.getRelativeValue(
				bodyConstants[renderedBodies[i].bodyConstantIdx].orbitalPeriod,
				bodyConstants[renderedBodies[earthIdx].bodyConstantIdx].orbitalPeriod, earthOrbitDelay));
		}
	}

	// calculate sum of all parents ascending node angle and sum of all inlinations including it's own
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		// not animated and not following other object
		if (renderedBodies[i].animatorIdx == -1 && renderedBodies[i].orbitParentIdx == -1) continue; 
		int parentIndex = renderedBodies[i].orbitParentIdx;
		renderedBodies[i].parentsAscendingNodeSum = m.sumAllAscendingNodes(
			renderedBodies, bodyConstants, parentIndex);
		renderedBodies[i].allInclinationSum = m.sumAllInclinations(
			renderedBodies, bodyConstants, i);
	}

	// initialize animators
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (renderedBodies[i].animatorIdx == -1) continue; // not animated
		int animatorIndex = renderedBodies[i].animatorIdx;
		int bodyConstantIndex = renderedBodies[i].bodyConstantIdx;
		animators.push_back(
			OrbitAnimator(delays[animatorIndex], bodyConstants[bodyConstantIndex].localOrbitalPeriod,
				renderedBodies[i].ovalRatio, renderedBodies[i].orbitRadius,	renderedBodies[i].allInclinationSum)
		);
	}

	randomizeOrbitAngles();

	cout << "Scene Set up\n";
	cout << "\nLoading Time: " << (int)glfwGetTime() - startLoadingTime << "s\n\n";


	// ==================== RENDER LOOP =========================

	glUseProgram(skyShaderProgram);
	glUniform1i(glGetUniformLocation(skyShaderProgram, "skybox"), 0); // set texture to 0

	glUseProgram(earthShaderProgram);
	glUniform1i(glGetUniformLocation(earthShaderProgram, "Texture1"), 0);
	glUniform1i(glGetUniformLocation(earthShaderProgram, "Texture2"), 1);
	glUniform1i(glGetUniformLocation(earthShaderProgram, "Texture3"), 2);

	sceneState.addSPlayTime(glfwGetTime());		// add asset loading time to paused time (rectify animation time)
	//sceneState.pauseScene(glfwGetTime(), true);

	glm::vec3 Xaxis = glm::vec3(1.f, 0.f, 0.f);
	glm::vec3 Yaxis = glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 Zaxis = glm::vec3(0.f, 0.f, 1.f);

	float ptime = glfwGetTime();
	float ftime = ptime;
	int fpsCount = 0;

	while (!glfwWindowShouldClose(window))
	{
		// input
		processKeyboard(window);

		// cap fps 
		if (!frameTrigger.toggle(glfwGetTime()))
		{
			continue;
		}
		ftime = glfwGetTime();
		if ((ftime - ptime) >= 10.f)
		{
			cout << "Avg FPS: " << fpsCount / (ftime - ptime) << endl;
			ptime = ftime;
			fpsCount = 0;
		}
		else
		{
			fpsCount++;
		}

		// animate animated objects (some object might just require rendering but not animating)
		if (sceneState.getCanUpdateAnimation())
		{
			float ms_time = (float)sceneState.getMsPlayTime(glfwGetTime()); // get animation time

			// animate all objects
			for (int i = 0; i < renderedBodies.size(); i++)
			{
				// if object is not aniamated and not orbiting anithing then skip
				if (renderedBodies[i].animatorIdx == -1) continue;

				// if object is not animated but orbiting something then it must be following it
				if (renderedBodies[i].animatorIdx == -1 && renderedBodies[i].orbitParentIdx != -1)
				{
					renderedBodies[i].position = renderedBodies[renderedBodies[i].orbitParentIdx].position;
					continue;
				}

				// current orbiting object's parent index
				int parentIdx = renderedBodies[i].orbitParentIdx;
				int animatorIdx = renderedBodies[i].animatorIdx;

				// parent must update its animated position before the child is, thus parent index < child index
				if (parentIdx > i)
				{
					cout << "revolved celestial bodies must be animated before the it's children\n";
					exit(-1);
				}

				// animate orbit of current object with a origin of parent's position
				animators[animatorIdx].animate(ms_time, 5, 5, sceneState.getJustStarted());

				// retrieve position
				renderedBodies[i].position = animators[animatorIdx].getOrbitPosition();
				renderedBodies[i].rotation = animators[animatorIdx].getSpinAngle();

			}

			sceneState.setJustStarted(false);
		}

		// render
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// view and perspective
		glm::mat4 model = glm::mat4(1.f);
		glm::mat4 view = glm::mat4(1.f);
		glm::mat4 projection = glm::mat4(1.f);
		view = glm::lookAt(camera.getPosition(), camera.getPosition() + camera.getFront(), camera.getUp());
		projection = glm::perspective(glm::radians(camera.getFOV()),
			(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 10.f, 400000.f);

		for (int i = 0; i < renderedBodies.size(); i++)
		{
			int bcIdx = renderedBodies[i].bodyConstantIdx;
			int txIdx = renderedBodies[i].textureIdx;
			BodyConst& bc = bodyConstants[bcIdx];
			RenderedBody& rb = renderedBodies[i];

			// if object is not animated and not following any other object
			if (renderedBodies[i].animatorIdx == -1 && renderedBodies[i].orbitParentIdx == -1)
			{
				// if object is light source use different shader, default to illum shader
				unsigned int shaderProg = illumShaderProgram;
				if (i == sunIdx) shaderProg = basicShaderProgram;

				glUseProgram(shaderProg);
				model = glm::mat4(1.f);
				model = glm::translate(model, vecToVec3(rb.position)); // sum all position
				model = glm::rotate(model, glm::radians(bc.axialTilt), Zaxis);
				model = glm::scale(model, glm::vec3(rb.scale));
				glSetModelViewProjection(shaderProg, model, view, projection);
				glDrawVertexTriangles(VAOs[rb.VAOIdx], textures[txIdx][0], vertexSize[rb.VAOIdx]);

			}
			// if object is animated or following animated object
			else
			{
				RenderedBody& pr = renderedBodies[rb.orbitParentIdx];
				
				// use special shader for earth
				if (i == earthIdx) glUseProgram(earthShaderProgram);
				else glUseProgram(illumShaderProgram);

				model = glm::mat4(1.f);

				// rotate using parents' ascending node to rectify orbit shift due to parent's shift of their own ascending node angle
				model = glm::rotate(model, glm::radians(rb.parentsAscendingNodeSum), Yaxis);

				// move world centered orbit to body centered orbit (such as moon orbiting sun translate to moon orbiting earth)
				model = glm::translate(model, vecToVec3(m.sumAllPositions(renderedBodies, rb.orbitParentIdx)));

				// rotate whole orbit to change orbit ascending starting point
				model = glm::rotate(model, glm::radians(bc.ascendingNode), Yaxis);

				// move orbit to world centered orbit position
				model = glm::translate(model, vecToVec3(rb.position));

				// create sphere tilt and add // TODO all parents' inclidnations
				model = glm::rotate(model, glm::radians(bc.axialTilt + rb.allInclinationSum), Zaxis);

				// create sphere spin
				model = glm::rotate(model, glm::radians(rb.rotation), Yaxis);
				
				// scale to correct size
				model = glm::scale(model, glm::vec3(rb.scale));			

				// store actual position for model view camera
				glm::vec4 actualPos = model * glm::vec4(0, 0, 0, 1);
				rb.finalPosition = vec3ToVec(
					glm::vec3(
						actualPos[0]/actualPos[3], 
						actualPos[1]/actualPos[3], 
						actualPos[2]/actualPos[3]
					)
				);

				// for earth use special shader
				if (i == earthIdx)
				{
					glSetLightingConfig(earthShaderProgram, lightPos, camera, fTrigger.getValue());
					glUniform1f(glGetUniformLocation(earthShaderProgram, "light[0].ambientStrength"), 0.06f);
					glSetModelViewProjection(earthShaderProgram, model, view, projection);
					glBindVertexArray(VAOs[rb.VAOIdx]);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, textures[txIdx][0]);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, textures[txIdx][1]);
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, textures[txIdx][2]);
					glDrawArrays(GL_TRIANGLES, 0, vertexSize[rb.VAOIdx]);
				}
				else
				{
					glSetLightingConfig(illumShaderProgram, lightPos, camera, fTrigger.getValue());
					glSetModelViewProjection(illumShaderProgram, model, view, projection);
					glDrawVertexTriangles(VAOs[rb.VAOIdx], textures[txIdx][0], vertexSize[rb.VAOIdx]);
				}
			}
		}

		// skybox (contains gl code)
		displaySkyBox(skyVAO, skyTexture, skyShaderProgram, view, projection);

		// return errors if there's any
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) { cout << "OpenGL Error Occured. Error Code: " << err << endl; }

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}


// ====================== general functions ========================

void processKeyboard(GLFWwindow* window)
{

	// window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

	// camera mode switching
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && qTrigger.toggle(glfwGetTime()))
	{
		while (true)
		{
			modelSelection -= 1;
			if (modelSelection == -1) modelSelection = renderedBodies.size() - 1;
			if (renderedBodies[modelSelection].modelViewed)	break;
		}
		if (!cameraMode) camera.setYaw(camera.getYaw() - 180.f); // fix camera flip 180
		cameraMode = true;

	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && eTrigger.toggle(glfwGetTime()))
	{
		while (true)
		{
			modelSelection += 1;
			if (modelSelection > renderedBodies.size() - 1) modelSelection = 0;
			if (renderedBodies[modelSelection].modelViewed)	break;
		}
		if (!cameraMode) camera.setYaw(camera.getYaw() - 180.f); // fix camera flip 180
		cameraMode = true;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && leftCtrlTrigger.toggle(glfwGetTime()))
	{
		if (cameraMode)
		{
			cameraMode = false;
			camera.setYaw(camera.getYaw() + 180.f); // fix camera flip 180 when switching back
			camera.orientCamera(0, 0);
		}
		else
		{
			cameraMode = true;
			camera.setYaw(camera.getYaw() - 180.f); // fix camera flip 180
		}
	}

	// fly through camera
	if (!cameraMode)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && wTrigger.toggle(glfwGetTime())) 
			camera.moveCamera(CameraMovement::FORWARD, wTrigger.getDiffRatio());
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && sTrigger.toggle(glfwGetTime()))	
			camera.moveCamera(CameraMovement::BACKWARD, sTrigger.getDiffRatio());
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && aTrigger.toggle(glfwGetTime()))	
			camera.moveCamera(CameraMovement::LEFT, aTrigger.getDiffRatio());
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && dTrigger.toggle(glfwGetTime()))	
			camera.moveCamera(CameraMovement::RIGHT, dTrigger.getDiffRatio());
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && spaceTrigger.toggle(glfwGetTime()))	
			camera.moveCamera(CameraMovement::UPWARD, spaceTrigger.getDiffRatio());
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && leftShiftTrigger.toggle(glfwGetTime())) 
			camera.moveCamera(CameraMovement::DOWNWARD, leftShiftTrigger.getDiffRatio());
	}
	// model view camera
	else
	{
		float x = 0.f, y = 0.f;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && dTrigger.toggle(glfwGetTime()))
		{
			x += 1.f * dTrigger.getDiffRatio();
			y += 0.f * dTrigger.getDiffRatio();
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && aTrigger.toggle(glfwGetTime()))
		{
			x += -1.f * aTrigger.getDiffRatio();
			y += 0.f * aTrigger.getDiffRatio();
		}
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && wTrigger.toggle(glfwGetTime()))
		{
			x += 0.f * wTrigger.getDiffRatio();
			y += -1.f * wTrigger.getDiffRatio();
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && sTrigger.toggle(glfwGetTime()))
		{
			x += 0.f * sTrigger.getDiffRatio();
			y += 1.f * sTrigger.getDiffRatio();
		}
		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS && zTrigger.toggle(glfwGetTime()))
		{
			float stepSize = powf(10, log10f(camera.getModelViewDistance()) - 1) * 0.05 * zTrigger.getDiffRatio();
			camera.increaseModelViewDistance(stepSize);
		}
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && cTrigger.toggle(glfwGetTime()))
		{
			float stepSize = powf(10, log10f(camera.getModelViewDistance()) - 1) * 0.05 * cTrigger.getDiffRatio();
			camera.decreaseModelViewDistance(stepSize);
		}
		camera.moveAndOrientCamera(
			vecToVec3(renderedBodies[modelSelection].finalPosition), x, y);
	}

	// general camera
	if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS && leftBracketTrigger.toggle(glfwGetTime())) 
		camera.decreaseFOV(0.05f * leftBracketTrigger.getDiffRatio());
	if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS && rightBracketTrigger.toggle(glfwGetTime()))
		camera.increaseFOV(0.05f * rightBracketTrigger.getDiffRatio());
	
		
	// pausing
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) 
		sceneState.pauseScene(glfwGetTime());
	
	// toggle player torch light
	if ((glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS))
		fTrigger.toggle(glfwGetTime());
	
	// speed up
	if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS && commaTrigger.toggle(glfwGetTime()))
	{
		float stepSize = powf(10, log10f(earthOrbitDelay) - 1) * 0.02 * commaTrigger.getDiffRatio();
		float newValue = earthOrbitDelay - stepSize;
		if (newValue > 0.01)
		{
			earthOrbitDelay = newValue;
			updateAnimatorsDelays();
			cout << "Earth 1yr = " << earthOrbitDelay << "s\n";
		}
	}
	// slow down
	if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS && periodTrigger.toggle(glfwGetTime()))
	{
		float stepSize = powf(10, log10f(earthOrbitDelay) - 1) * 0.02 * periodTrigger.getDiffRatio();
		float newValue = earthOrbitDelay + stepSize;
		if (newValue < 86401)
		{
			earthOrbitDelay = newValue;
			updateAnimatorsDelays();
			cout << "Earth 1yr = " << earthOrbitDelay << "s\n";
		}
	}

	// randomize orbit angle
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		// if toggled then trigger (the internal state doesn't matter)
		if (rTrigger.toggle(glfwGetTime()))
		{
			randomizeOrbitAngles();
		}
	}
}

void processMouse(GLFWwindow* window, double x, double y)
{
	if (firstMouse)
	{
		prevMouseX = x;
		prevMouseY = y;
		firstMouse = false;
	}

	float dX = x - prevMouseX;
	float dY = y - prevMouseY;

	prevMouseX = x;
	prevMouseY = y;

	if (!cameraMode)
	{
 		camera.orientCamera(dX, dY);
	}
}

unsigned int loadTexture(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		stbi_image_free(data);
		std::cout << "Texture Loaded: " << path << std::endl;
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// loads a cubemap texture from 6 individual texture faces
unsigned int loadCubemap(vector<string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	stbi_set_flip_vertically_on_load(true);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			GLenum format;
			if (nrChannels == 1)
				format = GL_RED;
			else if (nrChannels == 3)
				format = GL_RGB;
			else if (nrChannels == 4)
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}


// ============ animation calculations ================

void updateAnimatorsDelays()
{
	// calculate delays relative to earth orbiting period
	vector<float> delays;
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (renderedBodies[i].animatorIdx == -1) continue;
		if (i == earthIdx) // use delay specified by user
		{
			delays.push_back(earthOrbitDelay);
		}
		else // use delay relative to earth delay
		{
			delays.push_back(m.getRelativeValue(
				bodyConstants[renderedBodies[i].bodyConstantIdx].orbitalPeriod,
				bodyConstants[renderedBodies[earthIdx].bodyConstantIdx].orbitalPeriod, earthOrbitDelay));
		}
	}

	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (renderedBodies[i].animatorIdx == -1) continue; // not animated
		int animatorIndex = renderedBodies[i].animatorIdx;
		animators[animatorIndex].setOrbitalDelay(delays[animatorIndex]);
	}
}

void randomizeOrbitAngles()
{
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (renderedBodies[i].animatorIdx == -1) continue; // not animated
		int animatorIndex = renderedBodies[i].animatorIdx;
		int randAngle1 = rand() % 360;
		int randAngle2 = rand() % 360;
		animators[animatorIndex].setOrbitAngle(randAngle1); // randomize initial orbit angle
		if (renderedBodies[i].randomSpinAngle)
		{
			animators[animatorIndex].setSpinAngle(randAngle2);
		}
		else
		{
			animators[animatorIndex].setSpinAngle(
				randAngle1 + bodyConstants[renderedBodies[i].bodyConstantIdx].defaultSpinAngle);
		}
	}
}

// ============ extra openGL stuff ===============

void displayLoadingScreen(GLFWwindow* window)
{
	// this is not refactored and will never be
	vector<float> rectVert = getRectangle();
	GLuint loadingTexture = loadTexture("resources/loading_screen/loading.png");
	unsigned int loadingShaderProgram = LoadShader("shaders/load.vert", "shaders/load.frag");
	unsigned int loadVAO, loadVBO;
	glSetupVertexObject(loadVAO, loadVBO, rectVert, vector<int>{3, 2});

	glUseProgram(loadingShaderProgram);
	glm::mat4 lsModel = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f));
	glm::mat4 lsView = glm::lookAt(camera.getPosition(), camera.getPosition() + camera.getFront(), camera.getUp());
	glUniformMatrix4fv(glGetUniformLocation(loadingShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lsModel));
	glUniformMatrix4fv(glGetUniformLocation(loadingShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(lsView));

	glDrawVertexTriangles(loadVAO, loadingTexture, 6);
	glfwSwapBuffers(window);
}

void displaySkyBox(unsigned int& VAO, GLuint texture, unsigned int shaderProgram, glm::mat4 view, glm::mat4 projection)
{
	glDepthFunc(GL_LEQUAL);
	glUseProgram(shaderProgram);

	view = glm::mat4(glm::mat3(view)); // remove translation from view matrix
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default
}


void glSetupVertexObject(unsigned int& VAO, unsigned int& VBO, vector<float>& data, vector<int> attribLayout)
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);

	int strideCount = 0;
	for (int i = 0; i < attribLayout.size(); i++)
	{
		if (attribLayout[i] < 1) throw invalid_argument("glSetupVertexObject : attribute layout must be larger than 0");
		strideCount += attribLayout[i];
	}

	int offset = 0;
	for (int i = 0; i < attribLayout.size(); i++)
	{
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, attribLayout[i], GL_FLOAT, GL_FALSE, strideCount * sizeof(float), (void*)(offset * sizeof(float)));
		offset += attribLayout[i];
	}
}

void glDrawVertexTriangles(unsigned int VAO, GLuint texture, int numberOfVertex)
{
	glBindVertexArray(VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLES, 0, numberOfVertex);
}

void glSetModelViewProjection(unsigned int shaderProgram, glm::mat4 model, glm::mat4 view, glm::mat4 projection)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void glSetLightingConfig(unsigned int shaderProgram, glm::vec3 lightPos, GeneralCamera cam, int torch)
{
	glUniform3fv(glGetUniformLocation(shaderProgram, "light[0].position"), 1, &lightPos[0]);
	glUniform3f(glGetUniformLocation(shaderProgram, "light[0].color"), 1.f, 1.f, 1.f);
	glUniform3fv(glGetUniformLocation(shaderProgram, "light[0].camPos"), 1, &cam.getPosition()[0]);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[0].ambientStrength"), 0.15f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[0].specularStrength"), 0.3f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[0].shininess"), 16.f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[0].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[0].linear"), 0.000000014f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[0].quadratic"), 0.00000000007f);

	glUniform1i(glGetUniformLocation(shaderProgram, "torchLight"), torch);
	glUniform3fv(glGetUniformLocation(shaderProgram, "light[1].direction"), 1, &cam.getFront()[0]);
	glUniform3fv(glGetUniformLocation(shaderProgram, "light[1].position"), 1, &cam.getPosition()[0]);
	glUniform3f(glGetUniformLocation(shaderProgram, "light[1].color"), 1.f, 1.f, 1.f);
	glUniform3fv(glGetUniformLocation(shaderProgram, "light[1].camPos"), 1, &cam.getPosition()[0]);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].ambientStrength"), 0.f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].specularStrength"), 0.3f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].shininess"), 16.f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].linear"), 0.0000005f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].quadratic"), 0.000000015f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].phi"), 25.f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light[1].gamma"), 35.f);
}

glm::vec3 vecToVec3(vector<float> vec)
{
	return glm::vec3(vec[0], vec[1], vec[2]);
}

vector<float> vec3ToVec(glm::vec3 vec3)
{
	return vector<float>{vec3.x, vec3.y, vec3.z};
}

