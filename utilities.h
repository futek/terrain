#include <string>

#include <GL/glew.h>

std::string errorToString(GLenum error);
void errorPrint(std::string comment, GLenum error);
void errorCheck(std::string comment);