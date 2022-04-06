#pragma once
#include <GLFW/glfw3.h>

void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}

GLFWwindow* myCreateWindow(int w, int h, const char* title)
{
	// init glfw to initialize a window
	glfwInit();

	// specify OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// we don't need deprecated functions, so instead of 
	// compatibility profile, we use core profile
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// disable resize
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// create window
	GLFWwindow* window = glfwCreateWindow(w, h, title, NULL, NULL);
	
	// specify this in order to draw things
	glfwMakeContextCurrent(window);

	// allow us to specify a callback function whenever the window is resized
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	return window;
}

