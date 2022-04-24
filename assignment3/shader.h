#pragma once
#include <glad/glad.h>
#include "util.h"

unsigned int LoadShader(const char* vertexShaderFile, const char* fragmentShaderFile)
{
	int success;
	char infoLog[512];

	// create the shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	
	// load shader code as raw string
	char* vertexShaderSource = read_file(vertexShaderFile);
	
	// load shader source code into OpenGL
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);

	// compile shader code
	glCompileShader(vertexShader);

	// error checking in case compilation failed
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// same things as vertex shader just that this is fragment shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	char* fragmentShaderSource = read_file(fragmentShaderFile);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}


	// create shader program
	unsigned int shaderProgram = glCreateProgram();
	
	// attach shader to the program
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	
	// link shader program
	glLinkProgram(shaderProgram);


	// free memory from read_file
	free(vertexShaderSource);
	free(fragmentShaderSource);

	// remove shader after they are no longer needed
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}