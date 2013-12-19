#include "shader.h"

#include <cstdio>
#include <fstream>
#include <iostream>

const char *readFile(const char *path) {
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

bool compileShader(GLuint* shader, GLuint type, const char *path) {
	const char *source = readFile(path);

	*shader = glCreateShader(type);
	glShaderSource(*shader, 1, &source, NULL);
	glCompileShader(*shader);

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

	delete[] source;

	return true;
}

bool createShaderProgram(GLuint* program, std::string vertexShaderPath, std::string fragmentShaderPath, std::string outputAttribName) {
	*program = glCreateProgram();

	GLuint vertexShader, fragmentShader;
	compileShader(&vertexShader, GL_VERTEX_SHADER, vertexShaderPath.c_str());
	compileShader(&fragmentShader, GL_FRAGMENT_SHADER, fragmentShaderPath.c_str());

	glAttachShader(*program, vertexShader);
	glAttachShader(*program, fragmentShader);

	glBindFragDataLocation(*program, 0, outputAttribName.c_str());

	glLinkProgram(*program);

	GLint status;
	glGetProgramiv(*program, GL_LINK_STATUS, &status);
	if (!status) {
		std::cerr << "shader program failed to link:" << std::endl;
		GLint logSize;
		glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &logSize);
		char *logMsg = new char[logSize];
		glGetProgramInfoLog(*program, logSize, NULL, logMsg);
		std::cerr << logMsg << std::endl;
		delete[] logMsg;

		return false;
	}

	return true;
}