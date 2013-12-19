#include <string>

#include <GL/glew.h>

class Shader {
public:
	Shader(std::string vsp, std::string fsp, std::string oan) : vertexShaderPath(vsp), fragmentShaderPath(fsp), outputAttribName(oan) {};

	bool load();
	GLuint getUniformLocation(const char* name);
	GLuint getAttribLocation(const char* name);
	void use();

private:
	GLuint shaderProgram;
	std::string vertexShaderPath;
	std::string fragmentShaderPath;
	std::string outputAttribName;

	const char* read(const char *path);
	bool compile(GLuint* shader, GLuint type, const char *path);
	bool link();
};

