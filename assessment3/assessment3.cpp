#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

#include "camera.h"
#include "FlyThroughCamera.h"
#include "shader.h"
#include "window.h"
//#include "skybox.h"
#include "shapes.h"

using namespace std;

// prototypes
void processKeyboard(GLFWwindow* window);
void processMouse(GLFWwindow* window, double x, double y);
unsigned int loadCubemap(const char* filename[6]);

// settings
int window_width = 1280;
int window_height = 800;

// camera and camera control
SCamera Camera;
bool firstMouse = true;
float prevMouseX;
float prevMouseY;

int main(int argc, char** argv)
{
	// create window
	GLFWwindow* window = myCreateWindow(window_width, window_height, "Fly Through Camera");

	// disable mouse and set mouse event callback to processMourse function
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, processMouse);

	// init glad
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	// load shaders
	unsigned int cubeShaderProgram = LoadShader("mvp.vert", "col.frag");
	unsigned int skyShaderProgram = LoadShader("sky.vert", "sky.frag");
	
	// init camera
	InitCamera(Camera);

	// load skybox textures
	const char* files[6] = {
		"skybox/right.jpg",
		"skybox/left.jpg",
		"skybox/top.jpg",
		"skybox/bottom.jpg",
		"skybox/front.jpg",
		"skybox/back.jpg"
	};
	GLuint cubemapTexture = loadCubemap(files);
	
	// cube
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// sky
	unsigned int skyVAO, skyVBO;

	glGenVertexArrays(1, &skyVAO);
	glGenBuffers(1, &skyVBO);

	glBindVertexArray(skyVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// remove binding
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);

	// use shader
	glUseProgram(cubeShaderProgram);
	glUseProgram(skyShaderProgram);
	glUniform1i(glGetUniformLocation(skyShaderProgram, "skybox"), 0); // idk

	// render loop
	while (!glfwWindowShouldClose(window))
	{

		// input
		processKeyboard(window);

		// render
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// models
		glUseProgram(cubeShaderProgram);
		
		glm::vec3 cube_pos = glm::vec3(0.0f, 0.0f, 0.0f);

		glm::mat4 model = glm::mat4(1.f);
		model = glm::translate(model, cube_pos);
		glUniformMatrix4fv(glGetUniformLocation(cubeShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		glm::mat4 view = glm::mat4(1.f);
		view = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
		glUniformMatrix4fv(glGetUniformLocation(cubeShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 projection = glm::mat4(1.f);
		projection = glm::perspective(glm::radians(Camera.FOV), (float)window_width / (float)window_height, 1.f, 10.f);
		glUniformMatrix4fv(glGetUniformLocation(cubeShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		// skybox
		glDepthFunc(GL_LEQUAL);
		glUseProgram(skyShaderProgram);
		
		view = glm::mat4(glm::mat3(view)); // remove translation from view matrix
		glUniformMatrix4fv(glGetUniformLocation(skyShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(skyVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional clean up
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &skyVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &skyVBO);

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

unsigned int loadCubemap(const char* faces[6])
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < 6; i++)
	{
		unsigned char* data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
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