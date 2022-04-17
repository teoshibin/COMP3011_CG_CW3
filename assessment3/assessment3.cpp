
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <map>
#include "stb_image.h"
#include "camera.h"
#include "FlyThroughCamera.h"
#include "shader.h"
#include "window.h"
#include "modelReader.h"
#include "shapes.h"
#include "OrbitAnimator.h"
#include "SceneState.h"
#include "PlanetMath.h"

using namespace std;

void processKeyboard(GLFWwindow* window);
void processMouse(GLFWwindow* window, double x, double y);
unsigned int loadCubemap(vector<string> filename);
unsigned int loadTexture(const char* filename);
void displayLoadingScreen(GLFWwindow* window);
void displaySkyBox(unsigned int& VAO, GLuint texture, unsigned int shaderProgram, glm::mat4 view, glm::mat4 projection);
void glSetupVertexObject(unsigned int& VAO, unsigned int& VBO, vector<float>& data, vector<int> attribLayout);
void glDrawVertexTriangles(unsigned int VAO, GLuint texture, int numberOfVertex);
void glSetModelViewProjection(unsigned int shaderProgram, glm::mat4 model, glm::mat4 view, glm::mat4 projection);
void glSetLightingConfig(unsigned int shaderProgram, glm::vec3 lightPos, glm::vec3 camPos);

glm::vec3 vecToVec3(vector<float> vec);
vector<float> vec3ToVec(glm::vec3 vec3);

//void updateAnimatorsDelays(vector<OrbitAnimator&> animators);

// settings
int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 800;

// camera and camera control
SCamera Camera;
bool firstMouse = true;
float prevMouseX;
float prevMouseY;

// scene 
SceneState sceneState;
vector<OrbitAnimator> animators;
float earthOrbitDelay = 100;

int main(int argc, char** argv)
{

	// ======================= SETUP ======================		
	GLFWwindow* window = myCreateWindow(
		WINDOW_WIDTH, WINDOW_HEIGHT, "Space Scene");	// create window
	mySetWindowCenter(window);							// adjust window position
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // disable mouse
	glfwSetCursorPosCallback(window, processMouse);		// set mouse event callback
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); // init glad

	// ====================== OPENGL ======================		
	InitCamera(Camera);				// init camera
	displayLoadingScreen(window);	// loading screen (contains gl code)

	int startLoadingTime = (int)glfwGetTime(); // to calculate loading time


	// ========= load objects =========
	ObjFileReader ofr;
	ObjectFileData sphereObj, ufoObj;
	cout << "Loading Objects...\n";
	try
	{
		sphereObj = ofr.read("objects/solar_system/sphere.obj");
		ufoObj = ofr.read("objects/ufo_1/ufo_1.obj");
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
	cout << "Objects Loaded\n\n";


	// ======== load shaders =========
	cout << "Loading Shaders...\n";
	unsigned int illumShaderProgram = LoadShader("illuminated.vert", "illuminated.frag");
	unsigned int basicShaderProgram = LoadShader("basic.vert", "basic.frag");
	unsigned int skyShaderProgram = LoadShader("sky.vert", "sky.frag");
	cout << "Shaders Loaded\n\n";

	// ======= load all textures =======
	cout << "Loading Textures...\n";
	GLuint sunTexture = loadTexture("objects/solar_system/textures/2k_sun.jpg");	
	GLuint mercuryTexture = loadTexture("objects/solar_system/textures/2k_mercury.jpg");
	GLuint venusTexture = loadTexture("objects/solar_system/textures/2k_venus_surface.jpg");
	GLuint venusAtmosphereTexture = loadTexture("objects/solar_system/textures/2k_venus_atmosphere.jpg");
	GLuint earthTexture = loadTexture("objects/solar_system/textures/2k_earth_daymap.jpg");
	GLuint earthNightTexture = loadTexture("objects/solar_system/textures/2k_earth_nightmap.jpg");
	GLuint earthCloudsTexture = loadTexture("objects/solar_system/textures/2k_earth_clouds.jpg");
	GLuint moonTexture = loadTexture("objects/solar_system/textures/2k_moon.jpg");	
	GLuint marsTexture = loadTexture("objects/solar_system/textures/2k_mars.jpg");	
	GLuint jupiterTexture = loadTexture("objects/solar_system/textures/2k_jupiter.jpg");	
	GLuint saturnTexture = loadTexture("objects/solar_system/textures/2k_saturn.jpg");
	GLuint saturnRingTexture = loadTexture("objects/solar_system/textures/2k_saturn_ring_alpha.png");
	GLuint uranusTexture = loadTexture("objects/solar_system/textures/2k_uranus.jpg");
	GLuint uranusRingTexture = loadTexture("objects/solar_system/textures/uranus_ring.jpg");	
	GLuint neptuneTexture = loadTexture("objects/solar_system/textures/2k_neptune.jpg");
	GLuint plutoTexture = loadTexture("objects/solar_system/textures/pluto.jpg");
	GLuint ufoTexture = loadTexture("objects/ufo_1/ufo_kd.jpg");
	vector<string> files = {
		"skybox/right.jpg",
		"skybox/left.jpg",
		"skybox/bottom.jpg",
		"skybox/top.jpg",
		"skybox/front.jpg",
		"skybox/back.jpg"
	};
	GLuint skyTexture = loadCubemap(files);
	cout << "Textures Loaded\n\n";


	// ======= prepre scene rendering =======
	cout << "Setting Up Scene...\n";

	// gen buffers
	unsigned int ufoVAO, ufoVBO;
	glSetupVertexObject(ufoVAO, ufoVBO, ufoVert, vector<int>{3,2,3});	
	unsigned int sphereVAO, sphereVBO;
	glSetupVertexObject(sphereVAO, sphereVBO, sphereVert, vector<int>{3,2,3});
	unsigned int skyVAO, skyVBO;
	glSetupVertexObject(skyVAO, skyVBO, skyboxVert, vector<int>{3});

	// remove binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);

	// =========== MODEL & ANIMATION CONFIG ==============
	
	PlanetMath m;

	// model constants
	float SPHERE_OBJECT_RADIUS = 2;		// 3d sphere radius designed in blender (do not change)
	
	// model hyper params				(tweak these to adjust scene)
	
	float distanceModifier = 10;		// master distance margin scale
	float earthScale = 4;				// master scale

	// all these values have to change if customization rows are changed
	int attributeCount = 6;
	int earthIdx = 3;
	vector<float> bodiesCustomization{

		// 1. is animated boolean
		// 2. scale
		// 3. margin, the distance between the previous orbiting planet and current planet (this shouldn't include their radius)
		// 4. oval ratio, major minor axis ratio
		// 5. orbiting parent index (-1 not orbiting anything)
		// 6. index to body constant attributes (BodyConst class)

		// rules: none orbiting first and revovled object must come before orbiting object as some values are depend on orbited objects

//      1   2		3		4		5		6
		0,	0.5,	0,		1.f,	-1,		0,		// sun
		1,	1,		10,		1.f,	0,		1,		// mercury
		1,	1,		10,		1.f,	0,		2,		// venus
		1,	1,		10,		1.f,	0,		3,		// earth
		1,	1,		10,		1.f,	0,		4,		// mars
		1,	0.5,	80,		1.f,	0,		5,		// jupiter
		1,	0.5,	80,		1.f,	0,		6,		// saturn
		1,	1,		120,	1.f,	0,		7,		// uranus
		1,	1,		100,	1.f,	0,		8,		// neptune
		1,	1,		60,		1.f,	0,		9,		// pluto
		1,	1,		2,		1.f,	3,		10,		// moon
	};
	
	vector<RenderedBody> renderedBodies(bodiesCustomization.size() / attributeCount);
	
	// get predefined body constants
	vector<BodyConst> bc = m.getSolarSystemConstants();

	// add additional body constants here

			
	if (bodiesCustomization.size() / attributeCount != bc.size()) cout << "customization params not equal size\n";

	int animIndexCount = 0;
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		renderedBodies[i].animatorIndex = bodiesCustomization[i * attributeCount] ? animIndexCount : -1;
		animIndexCount++;
		renderedBodies[i].scaleModifier = bodiesCustomization[i * attributeCount + 1];
		renderedBodies[i].distanceMargin = bodiesCustomization[i * attributeCount + 2];
		renderedBodies[i].ovalRatio = bodiesCustomization[i * attributeCount + 3];
		renderedBodies[i].orbitParentIdx = bodiesCustomization[i * attributeCount + 4];
		renderedBodies[i].bodyConstantIdx = bodiesCustomization[i * attributeCount + 5];
	}

	//float sunScaleModifier = 0.5;		// specifically adjust body size
	//float mercuryScaleModifier = 1;
	//float venusScaleModifier = 1;
	//float moonScaleModifier = 1;
	//float marsScaleModifier = 1;
	//float jupiterScaleModifier = 0.5;
	//float saturnScaleModifier = 0.5;
	//float uranusScaleModifier = 1;
	//float neptuneScaleModifier = 1;
	//float plutoScaleModifier = 1;

	//float mercuryDistanceMargin = 10;	// set all to 0 will make them line up and touching each other
	//float venusDistanceMargin	= 10;    // (added distance)
	//float earthDistanceMargin	= 10;
	//float moonDistanceMargin = 2;
	//float marsDistanceMargin = 10;
	//float jupiterDistanceMargin = 80;
	//float saturnDistanceMargin = 80;
	//float uranusDistanceMargin = 120;
	//float neptuneDistanceMargin = 100;
	//float plutoDistanceMargin	= 60;

	// compute scale relative to earth
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (i == earthIdx) continue;
		renderedBodies[i].scale = m.getRelativeValue(bc[i].radius,
			PConst::EARTH_RADIUS, earthScale, renderedBodies[i].scaleModifier);
	}
	renderedBodies[earthIdx].scale = earthScale; // set earth scale


	//float sunScale = m.getRelativeValue(PConst::SUN_RADIUS, PConst::EARTH_RADIUS, earthScale, sunScaleModifier);
	//float mercuryScale = m.getRelativeValue(PConst::MERCURY_RADIUS, PConst::EARTH_RADIUS, earthScale, mercuryScaleModifier);
	//float venusScale = m.getRelativeValue(PConst::VENUS_RADIUS, PConst::EARTH_RADIUS, earthScale, venusScaleModifier);
	//float moonScale = m.getRelativeValue(PConst::MOON_RADIUS, PConst::EARTH_RADIUS, earthScale, moonScaleModifier);
	//float marsScale = m.getRelativeValue(PConst::MARS_RADIUS, PConst::EARTH_RADIUS, earthScale, marsScaleModifier);
	//float jupiterScale = m.getRelativeValue(PConst::JUPITER_RADIUS, PConst::EARTH_RADIUS, earthScale, jupiterScaleModifier);
	//float saturnScale = m.getRelativeValue(PConst::SATURN_RADIUS, PConst::EARTH_RADIUS, earthScale, saturnScaleModifier);
	//float uranusScale = m.getRelativeValue(PConst::URANUS_RADIUS, PConst::EARTH_RADIUS, earthScale, uranusScaleModifier);
	//float neptuneScale = m.getRelativeValue(PConst::NEPTUNE_RADIUS, PConst::EARTH_RADIUS, earthScale, neptuneScaleModifier);
	//float plutoScale = m.getRelativeValue(PConst::PLUTO_RADIUS, PConst::EARTH_RADIUS, earthScale, plutoScaleModifier);

	// compute virtual radius
	vector<float> sphereRadius;
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		sphereRadius.push_back(renderedBodies[i].scale * SPHERE_OBJECT_RADIUS);
	}

	// calculate orbit radius distance for individual bodies

	//// get unique parent index
	//vector<int>  uniqueParentIdx;
	//for (int i = 0; i < renderedBodies.size(); i++)
	//{
	//	// if no parent then abort
	//	if (renderedBodies[i].orbitParentIdx == -1) continue;

	//	// if empty then add index
	//	if (uniqueParentIdx.empty())
	//	{
	//		uniqueParentIdx.push_back(renderedBodies[i].orbitParentIdx);
	//	}
	//	else
	//	{
	//		// check if already exist within the array
	//		bool exist = false;
	//		for (int j = 0; j < uniqueParentIdx.size(); j++)
	//		{
	//			if (uniqueParentIdx[j] == renderedBodies[i].orbitParentIdx)
	//			{
	//				exist = true;
	//				break;
	//			}
	//		}
	//		if (!exist) uniqueParentIdx.push_back(renderedBodies[i].orbitParentIdx);
	//	}
	//}

	// use unique parent index to compute all orbit distance for all orbiting objects
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (renderedBodies[i].orbitParentIdx == -1) continue;

		// find last previous orbiting body index if doesn't exist then use parent index
		int preIdx = -1;
		for (int j = i - 1; j > -1; j--)
		{
			if (renderedBodies[i].orbitParentIdx == renderedBodies[j].orbitParentIdx)
			{
				preIdx = j;
				break;
			}
		}

		// calculate orbit radius, if this body is the first object that orbits it's parent then this
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
	//renderedBodies[m.getPlanet("moon")].orbitRadius = 
	//	sphereRadius[m.getPlanet("earth")] + sphereRadius[m.getPlanet("moon")] +
	//	(renderedBodies[m.getPlanet("moon")].distanceMargin * distanceModifier);


	//float sunRadius = sunScale * SPHERE_OBJECT_RADIUS;
	//float mercuryRadius = mercuryScale * SPHERE_OBJECT_RADIUS;
	//float venusRadius = venusScale * SPHERE_OBJECT_RADIUS;
	//float earthRadius = earthScale * SPHERE_OBJECT_RADIUS;
	//float marsRadius = marsScale * SPHERE_OBJECT_RADIUS;
	//float jupiterRadius = jupiterScale * SPHERE_OBJECT_RADIUS;
	//float saturnRadius = saturnScale * SPHERE_OBJECT_RADIUS;
	//float uranusRadius = uranusScale * SPHERE_OBJECT_RADIUS;
	//float neptuneRadius = neptuneScale * SPHERE_OBJECT_RADIUS;
	//float plutoRadius = plutoScale * SPHERE_OBJECT_RADIUS;
	//float moonRadius = moonScale * SPHERE_OBJECT_RADIUS;

	//float mercuryOrbitRadius = sunRadius + mercuryRadius + (mercuryDistanceMargin * distanceModifier);
	//float venusOrbitRadius = mercuryOrbitRadius + mercuryRadius + (venusDistanceMargin * distanceModifier);
	//float earthOrbitRadius = venusOrbitRadius + venusRadius + (earthDistanceMargin * distanceModifier);
	//float marsOrbitRadius = earthOrbitRadius + earthRadius + (marsDistanceMargin * distanceModifier);
	//float jupiterOrbitRadius = marsOrbitRadius + marsRadius + (jupiterDistanceMargin * distanceModifier);
	//float saturnOrbitRadius = jupiterOrbitRadius + jupiterRadius + (saturnDistanceMargin * distanceModifier);
	//float uranusOrbitRadius = saturnOrbitRadius + saturnRadius + (uranusDistanceMargin * distanceModifier);
	//float neptuneOrbitRadius = uranusOrbitRadius + uranusRadius + (neptuneDistanceMargin * distanceModifier);
	//float plutoOrbitRadius = neptuneOrbitRadius + neptuneRadius + (plutoDistanceMargin * distanceModifier);	
	//float moonOrbitRadius = earthRadius + moonRadius + (moonDistanceMargin * distanceModifier);
	
	// animation hyper params			(tweak these to adjust scene animation)

	int lowPrecision = 2;				// animation angle interpolation precision
	int midPrecision = 3;					
	int highPrecision = 5;
	//float earthOrbitDelay = 10;		// time for completing earth orbit animation in seconds (1yr = this amount of seconds)

	// calculate delays
	vector<float> delays;
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (renderedBodies[i].animatorIndex == -1) continue;
		if (i == earthIdx) // use delay specified by user
		{
			delays.push_back(earthOrbitDelay); 
		}
		else if (renderedBodies[i].bodyConstantIdx < 0) // use delay relative to earth delay
		{
			delays.push_back(m.getRelativeValue(
				bc[renderedBodies[i].bodyConstantIdx].orbitalPeriod, 
				bc[renderedBodies[earthIdx].bodyConstantIdx].orbitalPeriod, earthOrbitDelay));
		}
	}

	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (renderedBodies[i].animatorIndex == -1) continue;
		int aIdx = animators.size() - 1;
		animators.push_back(
			OrbitAnimator(delays[aIdx], bc[i].localOrbitalPeriod, 
				renderedBodies[i].ovalRatio, renderedBodies[i].orbitRadius, bc[i].inclination)
		);
	}

	// rectify moon's inclination as it is orbiting earth (adding earth's inclination into it's orbit animation math)
	animators[renderedBodies[m.getPlanet("moon")].animatorIndex].addOrbitAngle(bc[m.getPlanet("earth")].inclination);

	//// calculate moon time span using ratio (earthDelay / moonDelay = moonOrbitEarthDays / earthOrbitEarthDays)
	//float mercuryOrbitDelay = m.getRelativeValue(PConst::MERCURY_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
	//float venusOrbitDelay = m.getRelativeValue(PConst::VENUS_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
	//float marsOrbitDelay = m.getRelativeValue(PConst::MARS_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
	//float jupiterOrbitDelay = m.getRelativeValue(PConst::JUPITER_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
	//float saturnOrbitDelay = m.getRelativeValue(PConst::SATURN_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
	//float uranusOrbitDelay = m.getRelativeValue(PConst::URANUS_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
	//float neptuneOrbitDelay = m.getRelativeValue(PConst::NEPTUNE_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
	//float plutoOrbitDelay = m.getRelativeValue(PConst::PLUTO_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
	//float moonOrbitDelay = m.getRelativeValue(PConst::MOON_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);

	//OrbitAnimator mercuryOrbitor = OrbitAnimator(mercuryOrbitDelay, PConst::MERCURY_LOCAL_ORBITAL_PERIOD, 1.f, mercuryOrbitRadius, PConst::MERCURY_INCLINATION);
	//OrbitAnimator venusOrbitor = OrbitAnimator(venusOrbitDelay, PConst::VENUS_LOCAL_ORBITAL_PERIOD, 1.f, venusOrbitRadius, PConst::VENUS_INCLINATION);
	//OrbitAnimator earthOrbitor = OrbitAnimator(earthOrbitDelay, PConst::EARTH_ORBITAL_PERIOD, 1.f, earthOrbitRadius, PConst::EARTH_INCLINATION);
	//OrbitAnimator marsOrbitor = OrbitAnimator(marsOrbitDelay, PConst::MARS_LOCAL_ORBITAL_PERIOD, 1.f, marsOrbitRadius, PConst::MARS_INCLINATION);
	//OrbitAnimator jupiterOrbitor = OrbitAnimator(jupiterOrbitDelay, PConst::JUPITER_LOCAL_ORBITAL_PERIOD, 1.f, jupiterOrbitRadius, PConst::JUPITER_INCLINATION);
	//OrbitAnimator saturnOrbitor = OrbitAnimator(saturnOrbitDelay, PConst::SATURN_LOCAL_ORBITAL_PERIOD, 1.f, saturnOrbitRadius, PConst::SATURN_INCLINATION);
	//OrbitAnimator uranusOrbitor = OrbitAnimator(uranusOrbitDelay, PConst::URANUS_LOCAL_ORBITAL_PERIOD, 1.f, uranusOrbitRadius, PConst::URANUS_INCLINATION);
	//OrbitAnimator neptuneOrbitor = OrbitAnimator(neptuneOrbitDelay, PConst::NEPTUNE_LOCAL_ORBITAL_PERIOD, 1.f, neptuneOrbitRadius, PConst::NEPTUNE_INCLINATION);
	//OrbitAnimator plutoOrbitor = OrbitAnimator(plutoOrbitDelay, PConst::PLUTO_LOCAL_ORBITAL_PERIOD, 1.f, plutoOrbitRadius, PConst::PLUTO_INCLINATION);
	//OrbitAnimator moonOrbitor = OrbitAnimator(moonOrbitDelay, PConst::MOON_LOCAL_ORBITAL_PERIOD, 1.f, moonOrbitRadius, PConst::MOON_INCLINATION + PConst::EARTH_INCLINATION);
	

	// ==================== RENDER LOOP =========================

	glUseProgram(skyShaderProgram);
	glUniform1i(glGetUniformLocation(skyShaderProgram, "skybox"), 0); // set texture to 0

	cout << "Scene Set up\n";
	cout << "\nLoading Time: " << (int)glfwGetTime() - startLoadingTime << "s\n\n";



	sceneState.addSPlayTime(glfwGetTime());		// add asset loading time to paused time (rectify animation time)
	sceneState.pauseScene(glfwGetTime(), true);

	glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 0.0f);

	//vector<glm::vec3&> positions(renderedBodies.size());
	


	//glm::vec3 mercuryPos, venusPos, earthPos, marsPos, jupiterPos, saturnPos, uranusPos, neptunePos, plutoPos;
	//glm::vec3 moonPos;
	
	glm::vec3 Xaxis = glm::vec3(1.f, 0.f, 0.f);
	glm::vec3 Yaxis = glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 Zaxis = glm::vec3(0.f, 0.f, 1.f);

	bool firstTime = true;
	while (!glfwWindowShouldClose(window))
	{
		// input
		processKeyboard(window);

		// animations calculations
		if (!sceneState.getPause() || firstTime)
		{
			float ms_time = (float)sceneState.getMsPlayTime(glfwGetTime());
			
			for (int i = 0; i < renderedBodies.size() - 1; i++)
			{
				// find parent index
				int parentIdx = renderedBodies[i].orbitParentIdx;
				if (parentIdx != -1)
				{
					// use parent index to find parent
					RenderedBody& parent = renderedBodies[parentIdx];

					// use parent position to calculate self orbit position
					OrbitAnimator& currentAnimator = animators[renderedBodies[i].animatorIndex];
					currentAnimator.animate(
						parent.position, ms_time, 2, 5, firstTime);

					// save self orbit position to rendered bodies
					renderedBodies[i].position = currentAnimator.getOrbitPosition();
				}

			}

			//mercuryOrbitor.animate(vec3ToVec(sunPos), ms_time, lowPrecision, lowPrecision, firstTime);
			//venusOrbitor.animate(vec3ToVec(sunPos), ms_time, lowPrecision, lowPrecision, firstTime);
			//earthOrbitor.animate(vec3ToVec(sunPos), ms_time, lowPrecision, lowPrecision, firstTime);
			//marsOrbitor.animate(vec3ToVec(sunPos), ms_time, lowPrecision, lowPrecision, firstTime);
			//jupiterOrbitor.animate(vec3ToVec(sunPos), ms_time, lowPrecision, midPrecision, firstTime);
			//saturnOrbitor.animate(vec3ToVec(sunPos), ms_time, lowPrecision, midPrecision, firstTime);
			//uranusOrbitor.animate(vec3ToVec(sunPos), ms_time, lowPrecision, highPrecision, firstTime);
			//neptuneOrbitor.animate(vec3ToVec(sunPos), ms_time, lowPrecision, highPrecision, firstTime);
			//plutoOrbitor.animate(vec3ToVec(sunPos), ms_time, lowPrecision, highPrecision, firstTime);
			//mercuryPos = vecToVec3(mercuryOrbitor.getOrbitPosition());
			//venusPos = vecToVec3(venusOrbitor.getOrbitPosition());
			//earthPos = vecToVec3(earthOrbitor.getOrbitPosition());
			//marsPos = vecToVec3(marsOrbitor.getOrbitPosition());
			//jupiterPos = vecToVec3(jupiterOrbitor.getOrbitPosition());
			//saturnPos = vecToVec3(saturnOrbitor.getOrbitPosition());
			//uranusPos = vecToVec3(uranusOrbitor.getOrbitPosition());
			//neptunePos = vecToVec3(neptuneOrbitor.getOrbitPosition());
			//plutoPos = vecToVec3(plutoOrbitor.getOrbitPosition());

			//moonOrbitor.animate(vec3ToVec(earthPos), ms_time, lowPrecision, lowPrecision, firstTime);
			//moonPos = vecToVec3(moonOrbitor.getOrbitPosition());

			firstTime = false;
		}

		// render
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// view and perspective
		glm::mat4 model = glm::mat4(1.f);
		glm::mat4 view = glm::mat4(1.f);
		glm::mat4 projection = glm::mat4(1.f);
		view = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
		projection = glm::perspective(glm::radians(Camera.FOV), 
			(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 10000.f);

		// temp
		/*float sunScale = renderedBodies[m.getPlanet("sun")].scale;
		float mercyr*/

		// sphere - sun
		glUseProgram(basicShaderProgram);
		model = glm::mat4(1.f);
		model = glm::translate(model, lightPos);
		model = glm::rotate(model, glm::radians(PConst::SUN_AXIAL_TILT), Zaxis);
		model = glm::scale(model, glm::vec3(renderedBodies[m.getPlanet("sun")].scale));
		glSetModelViewProjection(basicShaderProgram, model, view, projection);
		glDrawVertexTriangles(sphereVAO, sunTexture, sphereVert.size() / 8);

		//// sphere - mercury
		//glUseProgram(illumShaderProgram);
		//model = glm::mat4(1.f);
		//model = glm::rotate(model, glm::radians(PConst::MERCURY_ASCENDING_NODE), Yaxis);
		//model = glm::translate(model, mercuryPos);
		//model = glm::rotate(model, glm::radians(PConst::MERCURY_AXIAL_TILT + PConst::MERCURY_INCLINATION), Zaxis);
		//model = glm::rotate(model, glm::radians(mercuryOrbitor.getSpinAngle()), Yaxis);
		//model = glm::scale(model, glm::vec3(mercuryScale));
		//glSetLightingConfig(illumShaderProgram, sunPos, Camera.Position);
		//glSetModelViewProjection(illumShaderProgram, model, view, projection);
		//glDrawVertexTriangles(sphereVAO, mercuryTexture, sphereVert.size() / 8);

		//// sphere - venus
		//glUseProgram(illumShaderProgram);
		//model = glm::mat4(1.f);
		//model = glm::rotate(model, glm::radians(PConst::VENUS_ASCENDING_NODE), Yaxis);
		//model = glm::translate(model, venusPos);
		//model = glm::rotate(model, glm::radians(PConst::VENUS_AXIAL_TILT + PConst::VENUS_INCLINATION), Zaxis);
		//model = glm::rotate(model, glm::radians(venusOrbitor.getSpinAngle()), Yaxis);
		//model = glm::scale(model, glm::vec3(venusScale));
		//glSetLightingConfig(illumShaderProgram, sunPos, Camera.Position);
		//glSetModelViewProjection(illumShaderProgram, model, view, projection);
		//glDrawVertexTriangles(sphereVAO, venusTexture, sphereVert.size() / 8);

		// sphere - earth
		RenderedBody& earthBody = renderedBodies[m.getPlanet("earth")];

		glUseProgram(illumShaderProgram);
		model = glm::mat4(1.f);
		model = glm::rotate(model, glm::radians(PConst::EARTH_ASCENDING_NODE), Yaxis);
		model = glm::translate(model, vecToVec3(earthBody.position));
		model = glm::rotate(model, glm::radians(PConst::EARTH_AXIAL_TILT + PConst::EARTH_INCLINATION), Zaxis);
		model = glm::rotate(model, glm::radians(animators[earthBody.animatorIndex].getSpinAngle()), Yaxis);
		model = glm::scale(model, glm::vec3(renderedBodies[m.getPlanet("earth")].scale));
		glSetLightingConfig(illumShaderProgram, lightPos, Camera.Position);
		glSetModelViewProjection(illumShaderProgram, model, view, projection);
		glDrawVertexTriangles(sphereVAO, earthTexture, sphereVert.size() / 8);

		//// sphere - mars
		//glUseProgram(illumShaderProgram);
		//model = glm::mat4(1.f);
		//model = glm::rotate(model, glm::radians(PConst::MARS_ASCENDING_NODE), Yaxis);
		//model = glm::translate(model, marsPos);
		//model = glm::rotate(model, glm::radians(PConst::MARS_AXIAL_TILT + PConst::MARS_INCLINATION), Zaxis);
		//model = glm::rotate(model, glm::radians(marsOrbitor.getSpinAngle()), Yaxis);
		//model = glm::scale(model, glm::vec3(marsScale));
		//glSetLightingConfig(illumShaderProgram, sunPos, Camera.Position);
		//glSetModelViewProjection(illumShaderProgram, model, view, projection);
		//glDrawVertexTriangles(sphereVAO, marsTexture, sphereVert.size() / 8);

		//// sphere - jupiter
		//glUseProgram(illumShaderProgram);
		//model = glm::mat4(1.f);
		//model = glm::rotate(model, glm::radians(PConst::JUPITER_ASCENDING_NODE), Yaxis);
		//model = glm::translate(model, jupiterPos);
		//model = glm::rotate(model, glm::radians(PConst::JUPITER_AXIAL_TILT + PConst::JUPITER_INCLINATION), Zaxis);
		//model = glm::rotate(model, glm::radians(jupiterOrbitor.getSpinAngle()), Yaxis);
		//model = glm::scale(model, glm::vec3(jupiterScale));
		//glSetLightingConfig(illumShaderProgram, sunPos, Camera.Position);
		//glSetModelViewProjection(illumShaderProgram, model, view, projection);
		//glDrawVertexTriangles(sphereVAO, jupiterTexture, sphereVert.size() / 8);

		//// sphere - saturn
		//glUseProgram(illumShaderProgram);
		//model = glm::mat4(1.f);
		//model = glm::rotate(model, glm::radians(PConst::SATURN_ASCENDING_NODE), Yaxis);
		//model = glm::translate(model, saturnPos);
		//model = glm::rotate(model, glm::radians(PConst::SATURN_AXIAL_TILT + PConst::SATURN_INCLINATION), Zaxis);
		//model = glm::rotate(model, glm::radians(saturnOrbitor.getSpinAngle()), Yaxis);
		//model = glm::scale(model, glm::vec3(saturnScale));
		//glSetLightingConfig(illumShaderProgram, sunPos, Camera.Position);
		//glSetModelViewProjection(illumShaderProgram, model, view, projection);
		//glDrawVertexTriangles(sphereVAO, saturnTexture, sphereVert.size() / 8);

		//// sphere - uranus
		//glUseProgram(illumShaderProgram);
		//model = glm::mat4(1.f);
		//model = glm::rotate(model, glm::radians(PConst::URANUS_ASCENDING_NODE), Yaxis);
		//model = glm::translate(model, uranusPos);
		//model = glm::rotate(model, glm::radians(PConst::URANUS_AXIAL_TILT + PConst::URANUS_INCLINATION), Zaxis);
		//model = glm::rotate(model, glm::radians(uranusOrbitor.getSpinAngle()), Yaxis);
		//model = glm::scale(model, glm::vec3(uranusScale));
		//glSetLightingConfig(illumShaderProgram, sunPos, Camera.Position);
		//glSetModelViewProjection(illumShaderProgram, model, view, projection);
		//glDrawVertexTriangles(sphereVAO, uranusTexture, sphereVert.size() / 8);

		//// sphere - neptune
		//glUseProgram(illumShaderProgram);
		//model = glm::mat4(1.f);
		//model = glm::rotate(model, glm::radians(PConst::NEPTUNE_ASCENDING_NODE), Yaxis);
		//model = glm::translate(model, neptunePos);
		//model = glm::rotate(model, glm::radians(PConst::NEPTUNE_AXIAL_TILT + PConst::NEPTUNE_INCLINATION), Zaxis);
		//model = glm::rotate(model, glm::radians(neptuneOrbitor.getSpinAngle()), Yaxis);
		//model = glm::scale(model, glm::vec3(neptuneScale));
		//glSetLightingConfig(illumShaderProgram, sunPos, Camera.Position);
		//glSetModelViewProjection(illumShaderProgram, model, view, projection);
		//glDrawVertexTriangles(sphereVAO, neptuneTexture, sphereVert.size() / 8);

		//// sphere - pluto
		//glUseProgram(illumShaderProgram);
		//model = glm::mat4(1.f);
		//model = glm::rotate(model, glm::radians(PConst::PLUTO_ASCENDING_NODE), Yaxis);
		//model = glm::translate(model, plutoPos);
		//model = glm::rotate(model, glm::radians(PConst::PLUTO_AXIAL_TILT + PConst::PLUTO_INCLINATION), Zaxis);
		//model = glm::rotate(model, glm::radians(plutoOrbitor.getSpinAngle()), Yaxis);
		//model = glm::scale(model, glm::vec3(plutoScale));
		//glSetLightingConfig(illumShaderProgram, sunPos, Camera.Position);
		//glSetModelViewProjection(illumShaderProgram, model, view, projection);
		//glDrawVertexTriangles(sphereVAO, plutoTexture, sphereVert.size() / 8);

		// sphere - moon
		// moon is slightly more complex as it is orbiting earth
		glUseProgram(illumShaderProgram);

		RenderedBody& moonBody = renderedBodies[m.getPlanet("moon")];

		model = glm::mat4(1.f);
		//model = glm::rotate(model, glm::radians(PConst::EARTH_ASCENDING_NODE + PConst::MOON_ASCENDING_NODE), Yaxis); // add dynamic ascending node here
		model = glm::rotate(model, glm::radians(PConst::EARTH_ASCENDING_NODE), Yaxis);
		model = glm::translate(model, vecToVec3(moonBody.position));
		model = glm::rotate(model, glm::radians(PConst::EARTH_INCLINATION + PConst::MOON_AXIAL_TILT + PConst::MOON_INCLINATION), Zaxis);
		model = glm::rotate(model, glm::radians(animators[moonBody.animatorIndex].getSpinAngle()), Yaxis);
		model = glm::scale(model, glm::vec3(moonBody.scale));
		glSetLightingConfig(illumShaderProgram, lightPos, Camera.Position);
		glSetModelViewProjection(illumShaderProgram, model, view, projection);
		glDrawVertexTriangles(sphereVAO, moonTexture, sphereVert.size() / 8);

		// ufo
		//glUseProgram(illumShaderProgram);
		//model = glm::mat4(1.f);
		//model = glm::translate(model, glm::vec3(earthPos.x, earthPos.y + earthScale*2, earthPos.z));
		//model = glm::scale(model, glm::vec3(earthScale/10.f));
		//glSetLightingConfig(illumShaderProgram, lightPos, Camera.Position);
		//glSetModelViewProjection(illumShaderProgram, model, view, projection);
		//glDrawVertexTriangles(ufoVAO, ufoTexture, ufoVert.size() / 8);


		// skybox (contains gl code)
		displaySkyBox(skyVAO, skyTexture, skyShaderProgram, view, projection);

		// return errors if there's any
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)	{ cout << "OpenGL Error Occured. Error Code: " << err << endl; }

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
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)		glfwSetWindowShouldClose(window, true);

	// camera
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)			MoveCamera(Camera, SCamera::FORWARD);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)			MoveCamera(Camera, SCamera::BACKWARD);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)			MoveCamera(Camera, SCamera::LEFT);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)			MoveCamera(Camera, SCamera::RIGHT);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)		MoveCamera(Camera, SCamera::UPWARD);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)	MoveCamera(Camera, SCamera::DOWNWARD);

	if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) Camera.FOV -= 0.05f;
	if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) Camera.FOV += 0.05f;

	// scene
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) sceneState.pauseScene(glfwGetTime());
	if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)
	{
		earthOrbitDelay -= 1;
		//updateAnimatorsDelays(animators);
		cout << "Earth 1yr = " << earthOrbitDelay << "s\n";
	}
	if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS)
	{
		earthOrbitDelay += 1;
		//updateAnimatorsDelays(animators);
		cout << "Earth 1yr = " << earthOrbitDelay << "s\n";
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

	OrientCamera(Camera, dX, dY);
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

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
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


// ============ extra openGL stuff ===============

void displayLoadingScreen(GLFWwindow* window)
{
	vector<float> rectVert = getRectangle();
	GLuint loadingTexture = loadTexture("objects/loading_screen/loading.png");
	unsigned int loadingShaderProgram = LoadShader("load.vert", "load.frag");
	unsigned int loadVAO, loadVBO;
	glSetupVertexObject(loadVAO, loadVBO, rectVert, vector<int>{3, 2});

	glUseProgram(loadingShaderProgram);
	glm::mat4 lsModel = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f));
	glm::mat4 lsView = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
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

void glSetLightingConfig(unsigned int shaderProgram, glm::vec3 lightPos, glm::vec3 camPos)
{
	//glUniform3fv(glGetUniformLocation(illumShaderProgram, "light.direction"), 1, &lightDirection[0]);
	glUniform3fv(glGetUniformLocation(shaderProgram, "light.position"), 1, &lightPos[0]);
	glUniform3f(glGetUniformLocation(shaderProgram, "light.color"), 1.f, 1.f, 1.f);
	glUniform3fv(glGetUniformLocation(shaderProgram, "light.camPos"), 1, &camPos[0]);
	glUniform1f(glGetUniformLocation(shaderProgram, "light.ambientStrength"), 0.2f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light.specularStrength"), 0.3f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light.shininess"), 16.f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light.linear"), 0.000014f);
	glUniform1f(glGetUniformLocation(shaderProgram, "light.quadratic"), 0.00000007f);
	//glUniform1f(glGetUniformLocation(shaderProgram, "light.phi"), 15.f);
}

glm::vec3 vecToVec3(vector<float> vec)
{
	return glm::vec3(vec[0], vec[1], vec[2]);
}

vector<float> vec3ToVec(glm::vec3 vec3)
{
	return vector<float>{vec3.x, vec3.y, vec3.z};
}

//void updateAnimatorsDelays(vector<OrbitAnimator&> animators)
//{
//	PlanetMath m;
//	vector<float> delays(animators.size(), 0.f);
//	delays[0] = m.getRelativeValue(PConst::MERCURY_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
//	delays[1] = m.getRelativeValue(PConst::VENUS_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
//	delays[2] = m.getRelativeValue(PConst::MARS_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
//	delays[3] = m.getRelativeValue(PConst::JUPITER_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
//	delays[4] = m.getRelativeValue(PConst::SATURN_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
//	delays[5] = m.getRelativeValue(PConst::URANUS_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
//	delays[6] = m.getRelativeValue(PConst::NEPTUNE_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
//	delays[7] = m.getRelativeValue(PConst::PLUTO_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
//	delays[8] = m.getRelativeValue(PConst::MOON_ORBITAL_PERIOD, PConst::EARTH_ORBITAL_PERIOD, earthOrbitDelay);
//	for (int i = 0; i < animators.size(); i++)
//	{
//		animators[i].setOrbitalDelay(delays[i]);
//	}
//}