
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
float earthOrbitDelay = 20;
glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 0.0f);

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
	vector<vector<GLuint>> textures{
		{ sunTexture },
		{ mercuryTexture },
		{ venusTexture, venusAtmosphereTexture },
		{ earthTexture, earthNightTexture, earthCloudsTexture },
		{ marsTexture },
		{ jupiterTexture },
		{ saturnTexture },
		{ uranusTexture },
		{ neptuneTexture },
		{ plutoTexture },
		{ moonTexture },
		{ ufoTexture },
		{ saturnRingTexture },
		{ uranusRingTexture },
	};

	// ======= prepre scene rendering =======
	cout << "Setting Up Scene...\n";

	// gen buffers
	unsigned int sphereVAO, sphereVBO;
	glSetupVertexObject(sphereVAO, sphereVBO, sphereVert, vector<int>{3, 2, 3});
	unsigned int ufoVAO, ufoVBO;
	glSetupVertexObject(ufoVAO, ufoVBO, ufoVert, vector<int>{3, 2, 3});
	unsigned int skyVAO, skyVBO;
	glSetupVertexObject(skyVAO, skyVBO, skyboxVert, vector<int>{3});

	vector<unsigned int> VAOs{
		sphereVAO,
		ufoVAO,
	};

	vector<int> vertexSize{
		(int)sphereVert.size() / 8,
		(int)ufoVert.size() / 8,
	};

	// remove binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);

	// =========== MODEL & ANIMATION CONFIG ==============

	PlanetMath m;

	// model constants
	float SPHERE_OBJECT_RADIUS = 2;		// 3d vertex sphere radius (do not change)

	// model hyper params				(tweak these to adjust scene)

	float distanceModifier = 10;		// master distance margin scale
	float earthScale = 10;				// master scale

	// all these values have to change if customization structure is changed
	int attributeCount = 7;
	int earthIdx = 3;
	int sunIdx = 0;
	vector<float> bodiesCustomization{

		// 1. is animated boolean
		// 2. scale
		// 3. margin, the distance between the previous orbiting planet and current planet (this shouldn't include their radius)
		// 4. oval ratio, major minor axis ratio
		// 5. orbiting parent index (-1 not orbiting anything)
		// 6. index to body constant attributes (BodyConst class) and textures
		// 7. VAO and vertex size index

		// rules: none orbiting first and revovled object must come before orbiting object as some values are depend on orbited objects

//      1   2		3		4		5		6		7
		0,	0.5,	0,		1.f,	-1,		0,		0,	// sun
		1,	1,		10,		1.f,	0,		1,		0,	// mercury
		1,	1,		10,		1.f,	0,		2,		0,	// venus
		1,	1,		10,		1.f,	0,		3,		0,	// earth
		1,	1,		10,		1.f,	0,		4,		0,	// mars
		1,	0.5,	80,		1.f,	0,		5,		0,	// jupiter
		1,	0.5,	80,		1.f,	0,		6,		0,	// saturn
		1,	1,		120,	1.f,	0,		7,		0,	// uranus
		1,	1,		100,	1.f,	0,		8,		0,	// neptune
		1,	1,		60,		1.f,	0,		9,		0,	// pluto
		1,	1,		2,		1.f,	3,		10,		0,	// moon
	};

	vector<RenderedBody> renderedBodies(bodiesCustomization.size() / attributeCount);
	renderedBodies[sunIdx].position = vec3ToVec(lightPos);

	// get predefined body constants
	vector<BodyConst> bodyConstants = m.getSolarSystemConstants();

	// add additional body constants here


	if (bodiesCustomization.size() / attributeCount != bodyConstants.size()) cout << "customization params not equal size\n";

	// parse configurations
	int animIndexCount = 0;
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if ((bool)bodiesCustomization[i * attributeCount])
		{
			renderedBodies[i].animatorIndex = animIndexCount;
			animIndexCount++;
		}
		else
		{
			renderedBodies[i].animatorIndex = -1;
		}
		renderedBodies[i].scaleModifier = bodiesCustomization[i * attributeCount + 1];
		renderedBodies[i].distanceMargin = bodiesCustomization[i * attributeCount + 2];
		renderedBodies[i].ovalRatio = bodiesCustomization[i * attributeCount + 3];
		renderedBodies[i].orbitParentIdx = bodiesCustomization[i * attributeCount + 4];
		renderedBodies[i].bodyConstantIdx = bodiesCustomization[i * attributeCount + 5];
		renderedBodies[i].VAOIdx = bodiesCustomization[i * attributeCount + 6];
	}

	// compute scale relative to earth
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (i == earthIdx) continue;
		renderedBodies[i].scale = m.getRelativeValue(bodyConstants[i].radius,
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
		if (renderedBodies[i].animatorIndex == -1) continue;
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

	// calculate sum of all parents ascending node angle
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (renderedBodies[i].animatorIndex == -1) continue; // not animated
		int parentIndex = renderedBodies[i].orbitParentIdx;
		renderedBodies[i].parentsAscendingNodeSum = m.sumAllAscendingNodes(
			renderedBodies, bodyConstants, parentIndex);
		renderedBodies[i].allInclinationSum = m.sumAllInclinations(
			renderedBodies, bodyConstants, i);
	}

	// initialize animators
	for (int i = 0; i < renderedBodies.size(); i++)
	{
		if (renderedBodies[i].animatorIndex == -1) continue; // not animated
		int animatorIndex = renderedBodies[i].animatorIndex;
		int bodyConstantIndex = renderedBodies[i].bodyConstantIdx;
		animators.push_back(
			OrbitAnimator(delays[animatorIndex], bodyConstants[bodyConstantIndex].localOrbitalPeriod,
				renderedBodies[i].ovalRatio, renderedBodies[i].orbitRadius,
				m.sumAllInclinations(renderedBodies, bodyConstants, i))
		);
	}


	// ==================== RENDER LOOP =========================

	glUseProgram(skyShaderProgram);
	glUniform1i(glGetUniformLocation(skyShaderProgram, "skybox"), 0); // set texture to 0

	cout << "Scene Set up\n";
	cout << "\nLoading Time: " << (int)glfwGetTime() - startLoadingTime << "s\n\n";



	sceneState.addSPlayTime(glfwGetTime());		// add asset loading time to paused time (rectify animation time)
	sceneState.pauseScene(glfwGetTime(), true);

	glm::vec3 Xaxis = glm::vec3(1.f, 0.f, 0.f);
	glm::vec3 Yaxis = glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 Zaxis = glm::vec3(0.f, 0.f, 1.f);

	bool firstTime = true;
	while (!glfwWindowShouldClose(window))
	{
		// input
		processKeyboard(window);

		// animate animated objects (some object might just require rendering but not animating)
		if (!sceneState.getPause() || firstTime)
		{
			float ms_time = (float)sceneState.getMsPlayTime(glfwGetTime()); // get animation time

			for (int i = 0; i < renderedBodies.size(); i++)
			{
				// skip none animated objects
				if (renderedBodies[i].animatorIndex == -1) continue;

				// current orbiting object's parent index
				int parentIdx = renderedBodies[i].orbitParentIdx;
				int animatorIdx = renderedBodies[i].animatorIndex;
				//std::vector<float>& parentPosition = renderedBodies[parentIdx].position;

				// parent must update its animated position before the child is, thus parent index < child index
				if (parentIdx > i)
				{
					cout << "revolved celestial bodies must be animated before the it's children\n";
					exit(-1);
				}

				// animate orbit of current object with a origin of parent's position
				animators[animatorIdx].animate(ms_time, 2, 5, firstTime);

				// retrieve position
				renderedBodies[i].position = animators[animatorIdx].getOrbitPosition();
				renderedBodies[i].rotation = animators[animatorIdx].getSpinAngle();

			}

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

		for (int i = 0; i < renderedBodies.size(); i++)
		{
			int bcIdx = renderedBodies[i].bodyConstantIdx;
			BodyConst& bc = bodyConstants[bcIdx];
			RenderedBody& rb = renderedBodies[i];

			// if object is not animated
			if (renderedBodies[i].animatorIndex == -1)
			{
				// if object is light source use different shader, default to illum shader
				unsigned int shaderProg = illumShaderProgram;
				if (i == sunIdx) shaderProg = basicShaderProgram;

				glUseProgram(shaderProg);
				model = glm::mat4(1.f);
				model = glm::translate(model, vecToVec3(rb.position));
				model = glm::rotate(model, glm::radians(bc.axialTilt), Zaxis);
				model = glm::scale(model, glm::vec3(rb.scale));
				glSetModelViewProjection(shaderProg, model, view, projection);
				glDrawVertexTriangles(VAOs[rb.VAOIdx], textures[bcIdx][0], vertexSize[rb.VAOIdx]);

			}
			// if object is animated
			else
			{
				RenderedBody& pr = renderedBodies[rb.orbitParentIdx];

				glUseProgram(illumShaderProgram);
				model = glm::mat4(1.f);

				// rotate using parents' ascending node to rectify orbit shift due to parent's shift of their own ascending node angle
				model = glm::rotate(model, glm::radians(rb.parentsAscendingNodeSum), Yaxis);

				// move world centered orbit to body centered orbit (such as moon orbiting sun translate to moon orbiting earth)
				model = glm::translate(model, vecToVec3(pr.position));

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
				glSetLightingConfig(illumShaderProgram, lightPos, Camera.Position);
				glSetModelViewProjection(illumShaderProgram, model, view, projection);
				glDrawVertexTriangles(VAOs[rb.VAOIdx], textures[bcIdx][0], vertexSize[rb.VAOIdx]);
				//TODO use multiple textures
			}
		}

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
	stbi_set_flip_vertically_on_load(true);

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
	// this is not refactored and will never be
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