#include "utilities.h"

#include <cstdio>

std::string errorToString(GLenum error) {
	std::string errorString;
	switch (error) {
		case GL_NO_ERROR:                      errorString = "GL_NO_ERROR";                      break;
		case GL_INVALID_ENUM:                  errorString = "GL_INVALID_ENUM";                  break;
		case GL_INVALID_VALUE:                 errorString = "GL_INVALID_VALUE";                 break;
		case GL_INVALID_OPERATION:             errorString = "GL_INVALID_OPERATION";             break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: errorString = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 errorString = "GL_OUT_OF_MEMORY";                 break;
		case GL_STACK_UNDERFLOW:               errorString = "GL_STACK_UNDERFLOW";               break;
		case GL_STACK_OVERFLOW:                errorString = "GL_STACK_OVERFLOW";                break;
	}
	return errorString;
}

void errorPrint(std::string comment, GLenum error) {
	std::printf("OpenGL error: %s @ %s\n", errorToString(error).c_str(), comment.c_str());
}

void errorCheck(std::string comment) {
	GLenum error;
	while ((error = glGetError()) != GL_NO_ERROR) {
		errorPrint(comment, error);
	}
}