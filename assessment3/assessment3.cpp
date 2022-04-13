// openGL and window libraries
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

// build in libraries
#include <iostream>
#include <vector>

// helper libraries
#include "stb_image.h"

// custom functions
#include "camera.h"
#include "FlyThroughCamera.h"
#include "shader.h"
#include "window.h"
#include "readModel.h"
#include "shapes.h"
#include "geoMetrics.h"

using namespace std;

// prototypes
void processKeyboard(GLFWwindow* window);
void processMouse(GLFWwindow* window, double x, double y);
unsigned int loadCubemap(vector<string> filename);
unsigned int loadTexture(const char* filename);
void displayLoadingScreen(GLFWwindow* window);

// settings
int window_width = 1280;
int window_height = 800;

// camera and camera control
SCamera Camera;
bool firstMouse = true;
float prevMouseX;
float prevMouseY;

vector<float> rectVert{
	-1.f, 1.f, 0.f,		0, 1,	//tl
	1.f, 1.f, 0.f,		1, 1,	//tr
	1.f, -1.f, 0.f,		1, 0,	//br

	-1.f, 1.f, 0.f,		0,1,	//tl
	1.f,  -1.f, 0.f,		1,0,	//br
	-1.f,  -1.f, 0.f,	0,0		//bl
};

int main(int argc, char** argv)
{
	// ============= SETUP ==============

	// create window
	GLFWwindow* window = myCreateWindow(window_width, window_height, "Space Scene");

	// adjust window position
	mySetWindowCenter(window);

	// disable mouse and set mouse event callback to processMourse function
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, processMouse);

	// init glad
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	// ============= OPENGL =============

	// init camera
	InitCamera(Camera);

	// loading screen (this uses opengl code camera must be init before this)
	displayLoadingScreen(window);

	// load objects
	ObjFileReader ofr;
	WholeObj sphereObj;
	WholeObj ufoObj;
	cout << "Loading Objects..." << endl;
	try
	{
		sphereObj = ofr.readObj("myObjects/sphere.obj");
		ufoObj = ofr.readObj("myObjects/ufo_1.obj");
	}
	catch (const std::exception& e)
	{
		cerr << "Fail to load object file" << endl;
		cerr << e.what() << endl;
		return -1;
	}
	vector<float> skyboxVert = getSkyboxCube();
	vector<float>& sphereVert = sphereObj.subObjects[0].expandedVertices;
	vector<float>& ufoVert = ufoObj.subObjects[0].expandedVertices;
	cout << "Objects Loaded" << endl;

	// load shaders
	unsigned int basicShaderProgram = LoadShader("basic.vert", "basic.frag");
	unsigned int skyShaderProgram = LoadShader("sky.vert", "sky.frag");
	glUseProgram(skyShaderProgram);
	glUniform1i(glGetUniformLocation(skyShaderProgram, "skybox"), 0); // set texture to 0
	
	// load all textures
	GLuint earthTexture = loadTexture("myObjects/earth.png");
	GLuint moonTexture = loadTexture("myObjects/moon.png");
	GLuint sunTexture = loadTexture("myObjects/sun.png");
	GLuint ufoTexture = loadTexture("myObjects/ufo_kd.jpg");
	vector<string> files = {
		"skybox/right.jpg",
		"skybox/left.jpg",
		"skybox/bottom.jpg",
		"skybox/top.jpg",
		"skybox/front.jpg",
		"skybox/back.jpg"
	};
	GLuint cubemapTexture = loadCubemap(files);

	// ufo	
	unsigned int ufoVAO, ufoVBO;
	glGenVertexArrays(1, &ufoVAO);
	glBindVertexArray(ufoVAO);

	glGenBuffers(1, &ufoVBO);
	glBindBuffer(GL_ARRAY_BUFFER, ufoVBO);
	glBufferData(GL_ARRAY_BUFFER, ufoVert.size() * sizeof(float), &ufoVert[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

	// sphere	
	unsigned int sphereVAO, sphereVBO;
	glGenVertexArrays(1, &sphereVAO);
	glBindVertexArray(sphereVAO);

	glGenBuffers(1, &sphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sphereObj.subObjects[0].expandedVertices.size() * sizeof(float), &sphereObj.subObjects[0].expandedVertices[0], GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

	// skybox
	unsigned int skyVAO, skyVBO;
	glGenVertexArrays(1, &skyVAO);
	glBindVertexArray(skyVAO);

	glGenBuffers(1, &skyVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
	glBufferData(GL_ARRAY_BUFFER, skyboxVert.size() * sizeof(float), &skyboxVert[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// remove binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);

	// use shader
	//glUseProgram(cubeShaderProgram);
	//glUseProgram(skyShaderProgram);	

	// =========== MODEL & ANIMATION CONFIG ==============
	
	UniversalConstants c;
	
	// model hyper params
	float earth_scale = 0.3; // make earth smaller
	float sun_scale_modifier = 0.05; // make sun smaller (not to actual scale)
	float distance_modifier = 7; // (not to actual scale)

	// model constants
	float sun_scale = c.SUN_RADIUS / c.EARTH_RADIUS * earth_scale * sun_scale_modifier;
	float earth_to_sun_distance = sun_scale * distance_modifier; // using diameter of the sun
	float moon_scale = c.MOON_RADIUS / c.EARTH_RADIUS * earth_scale;
	float moon_to_earth_distance = earth_scale * distance_modifier;

	// animation hyper params
	int precision = 2;
	float earth_orbit_time_span = 240; // time for completing earth orbit animation in seconds

	// animation constants and objects

		// calculate moon time span using ratio (moon_tp / earth_tp) = (moon_orbital_earth_days / orbital_earth_days)
	float moon_orbit_time_span = earth_orbit_time_span / c.EARTH_ORBITAL_PERIOD * c.MOON_EARTH_DAYS_ORBITAL_PERIOD;

	OrbitAnimator earthOrbitor = OrbitAnimator(
		earth_orbit_time_span, c.EARTH_ORBITAL_PERIOD, 1.5f, earth_to_sun_distance, c.EARTH_ECLIPTIC_INCLINATION);

	// note that the orbital period unit we're using here is the moon day not earth day
	// moon got the same rotation and orbit period so the same side will always be facing earth
	OrbitAnimator moonOrbitor = OrbitAnimator(
		moon_orbit_time_span, c.MOON_MOON_DAYS_ORBITAL_PERIOD, 1.1f, moon_to_earth_distance, c.MOON_ECLIPTIC_INCLINATION);


	// render loop
	while (!glfwWindowShouldClose(window))
	{

		// animations calculations
		float ms_time = (float) glfwGetTime() * 1000;
		earthOrbitor.animate(ms_time, precision, precision);
		vector<float> earth_vector_pos = earthOrbitor.getOrbitPosition();
		moonOrbitor.animate(earth_vector_pos, ms_time, precision, precision);
		vector<float> moon_vector_pos = moonOrbitor.getOrbitPosition();

		// input
		processKeyboard(window);

		// render
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// view and perspective
		glm::mat4 model = glm::mat4(1.f);
		glm::mat4 view = glm::mat4(1.f);
		glm::mat4 projection = glm::mat4(1.f);
		view = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
		projection = glm::perspective(glm::radians(Camera.FOV), (float)window_width / (float)window_height, 0.1f, 100.f);

		// sphere - earth
		glUseProgram(basicShaderProgram);				

		glm::vec3 earth_pos = glm::vec3(earth_vector_pos[0], earth_vector_pos[1], earth_vector_pos[2]);
		model = glm::mat4(1.f);
		model = glm::translate(model, earth_pos);
		model = glm::rotate(model, glm::radians(c.EARTH_AXIAL_TILT), glm::vec3(0.f, 0.f, 1.f));
		model = glm::rotate(model, glm::radians(earthOrbitor.getSpinAngle()), glm::vec3(0.f, 1.f, 0.f));
		model = glm::scale(model, glm::vec3(earth_scale));

		glUniformMatrix4fv(glGetUniformLocation(basicShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(basicShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(basicShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(sphereVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, earthTexture);
		glDrawArrays(GL_TRIANGLES, 0, sphereVert.size()/8);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);

		//// sphere - moon
		glUseProgram(basicShaderProgram);

		glm::vec3 moon_pos = glm::vec3(moon_vector_pos[0], moon_vector_pos[1], moon_vector_pos[2]);
		model = glm::mat4(1.f);
		model = glm::translate(model, moon_pos);
		model = glm::rotate(model, glm::radians(c.MOON_AXIAL_TILT), glm::vec3(0.f, 0.f, 1.f));
		model = glm::rotate(model, glm::radians(moonOrbitor.getSpinAngle()), glm::vec3(0.f, 1.f, 0.f));
		model = glm::scale(model, glm::vec3(moon_scale));

		glUniformMatrix4fv(glGetUniformLocation(basicShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(basicShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(basicShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(sphereVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, moonTexture);
		glDrawArrays(GL_TRIANGLES, 0, sphereVert.size()/8);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);

		// ufo
		glUseProgram(basicShaderProgram);

		model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(earth_pos.x, earth_pos.y + 0.8f, earth_pos.z));
		model = glm::scale(model, glm::vec3(earth_scale/10.f));
		glUniformMatrix4fv(glGetUniformLocation(basicShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(basicShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(basicShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(ufoVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ufoTexture);
		glDrawArrays(GL_TRIANGLES, 0, ufoVert.size() / 8);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);

		// sphere - sun
		glUseProgram(basicShaderProgram);

		glm::vec3 sun_pos = glm::vec3(0.0f, 0.0f, 0.0f);
		model = glm::mat4(1.f);
		model = glm::translate(model, sun_pos);
		model = glm::rotate(model, glm::radians(c.SUN_AXIAL_TILT), glm::vec3(0.f, 1.f, 0.f));
		model = glm::scale(model, glm::vec3(sun_scale));

		glUniformMatrix4fv(glGetUniformLocation(basicShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(basicShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(basicShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(sphereVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sunTexture);
		glDrawArrays(GL_TRIANGLES, 0, sphereVert.size()/8);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);


		// skybox
		glDepthFunc(GL_LEQUAL);
		glUseProgram(skyShaderProgram);
		
		view = glm::mat4(glm::mat3(view)); // remove translation from view matrix
		glUniformMatrix4fv(glGetUniformLocation(skyShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(skyVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default

		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)	{ cout << "OpenGL Error Occured. Error Code: " << err << endl; }

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processKeyboard(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)			MoveCamera(Camera, SCamera::FORWARD);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)			MoveCamera(Camera, SCamera::BACKWARD);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)			MoveCamera(Camera, SCamera::LEFT);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)			MoveCamera(Camera, SCamera::RIGHT);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)		MoveCamera(Camera, SCamera::UPWARD);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)	MoveCamera(Camera, SCamera::DOWNWARD);

	if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) Camera.FOV -= 0.05f;
	if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) Camera.FOV += 0.05f;
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

// ============ loading screen ===============

void displayLoadingScreen(GLFWwindow* window)
{
	GLuint loadingTexture = loadTexture("myObjects/loading.png");
	unsigned int loadingShaderProgram = LoadShader("load.vert", "load.frag");
	unsigned int loadVAO, loadVBO;
	glGenVertexArrays(1, &loadVAO);
	glBindVertexArray(loadVAO);
	glGenBuffers(1, &loadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, loadVBO);
	glBufferData(GL_ARRAY_BUFFER, rectVert.size() * sizeof(float), &rectVert[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glUseProgram(loadingShaderProgram);
	glm::mat4 lsModel = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f));
	glm::mat4 lsView = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
	glUniformMatrix4fv(glGetUniformLocation(loadingShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lsModel));
	glUniformMatrix4fv(glGetUniformLocation(loadingShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(lsView));

	glBindVertexArray(loadVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, loadingTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glfwSwapBuffers(window);
}