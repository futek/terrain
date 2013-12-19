#include <string>

#include <GL/glew.h>

bool createShaderProgram(GLuint* shaderProgram, std::string vertexShaderPath, std::string fragmentShaderPath, std::string outputAttribName);