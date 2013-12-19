#include "shader.h"

#include <cstdio>
#include <fstream>
#include <iostream>

bool Shader::load() {
	return link();
}

GLuint Shader::getUniformLocation(const char* name) {
	GLuint location = glGetUniformLocation(shaderProgram, name);
	if (location == GL_INVALID_INDEX) {
		std::printf("shader did not contain '%s' uniform\n", name);
	}
	return location;
}

GLuint Shader::getAttribLocation(const char* name) {
	GLuint location = glGetAttribLocation(shaderProgram, name);
	if (location == GL_INVALID_INDEX) {
		std::printf("shader did not contain '%s' uniform\n", name);
	}
	return location;
}

void Shader::use() {
	glUseProgram(shaderProgram);
}


const char* Shader::read(const char *path) {
	std::ifstream file(path, std::ios::binary);

	file.seekg(0, std::ios::end);
	int length = file.tellg();
	file.seekg(0, std::ios::beg);

	char *source = new char[length + 1];

	file.read(source, length);
	file.close();

	source[length] = '\0';

	return source;
}

bool Shader::compile(GLuint* shader, GLuint type, const char *path) {
	const char* source = read(path);

	*shader = glCreateShader(type);
	glShaderSource(*shader, 1, &source, NULL);
	glCompileShader(*shader);

	delete[] source;

	GLint status;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		std::cerr << path << " failed to compile:" << std::endl;
		GLint logSize;
		glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logSize);
		char *logMsg = new char[logSize];
		glGetShaderInfoLog(*shader, logSize, NULL, logMsg);
		std::cerr << logMsg << std::endl;
		delete[] logMsg;

		return false;
	}

	return true;
}

bool Shader::link() {
	GLuint vertexShader, fragmentShader;
	bool success;

	success = compile(&vertexShader, GL_VERTEX_SHADER, vertexShaderPath.c_str());
	if (!success) return false;

	success = compile(&fragmentShader, GL_FRAGMENT_SHADER, fragmentShaderPath.c_str());
	if (!success) return false;

	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glBindFragDataLocation(shaderProgram, 0, outputAttribName.c_str());

	glLinkProgram(shaderProgram);

	GLint status;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
	if (!status) {
		std::cerr << "shader program failed to link:" << std::endl;
		GLint logSize;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize);
		char *logMsg = new char[logSize];
		glGetProgramInfoLog(shaderProgram, logSize, NULL, logMsg);
		std::cerr << logMsg << std::endl;
		delete[] logMsg;

		return false;
	}

	return true;
}