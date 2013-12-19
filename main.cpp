#include <cstdlib>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "shader.h"
#include "utilities.h"

using namespace glm;

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

GLFWwindow* window;
int width = WINDOW_WIDTH, height = WINDOW_HEIGHT;
bool fullscreen = false;

Shader terrainShader("shaders/terrain.vert", "shaders/terrain.frag", "outColor");
struct terrainShaderUniform {
	GLuint projectionMatrix;
	GLuint modelViewMatrix;
	GLuint time;
	GLuint playerPosition;
	GLuint fogColor;
	GLuint viewDistance;
} terrainShaderUniform;
GLuint terrainVertexArrayObject;

Shader waterShader("shaders/water.vert", "shaders/water.frag", "outColor");
struct waterShaderUniform {
	GLuint projectionMatrix;
	GLuint modelViewMatrix;
	GLuint time;
	GLuint playerPosition;
	GLuint fogColor;
	GLuint viewDistance;
} waterShaderUniform;
GLuint waterVertexArrayObject;

mat4 projectionMatrix, modelViewMatrix;

struct vertex {
	vec3 position;
};

const int gridWidth = 1500, gridHeight = 1500;
const GLuint numberOfVertices = gridWidth * gridHeight;
vertex vertices[numberOfVertices];
const GLuint numberOfIndices = numberOfVertices + (gridWidth - 1) * (gridHeight - 2);
int indices[numberOfIndices];

vec3 fogColor = vec3(225.0f, 240.0f, 245.0f) / 255.0f;
float viewDistance = min(gridWidth, gridHeight) / 2.0;

struct player {
	vec3 position;
	vec3 direction;
	vec3 up;
	float speed;
} player;

int keyTabPrev = 0;

void initialize();

bool loadShaders() {
	bool success;

	success = terrainShader.load();
	if (!success) return false;
	terrainShader.use();
	terrainShaderUniform.projectionMatrix = terrainShader.getUniformLocation("projection");
	terrainShaderUniform.modelViewMatrix  = terrainShader.getUniformLocation("modelView");
	terrainShaderUniform.time             = terrainShader.getUniformLocation("time");
	terrainShaderUniform.playerPosition   = terrainShader.getUniformLocation("playerPosition");
	terrainShaderUniform.fogColor         = terrainShader.getUniformLocation("fogColor");
	terrainShaderUniform.viewDistance     = terrainShader.getUniformLocation("viewDistance");
	glUseProgram(0);

	success = waterShader.load();
	if (!success) return false;
	waterShader.use();
	waterShaderUniform.projectionMatrix = waterShader.getUniformLocation("projection");
	waterShaderUniform.modelViewMatrix  = waterShader.getUniformLocation("modelView");
	waterShaderUniform.time             = waterShader.getUniformLocation("time");
	waterShaderUniform.playerPosition   = waterShader.getUniformLocation("playerPosition");
	waterShaderUniform.fogColor         = waterShader.getUniformLocation("fogColor");
	waterShaderUniform.viewDistance     = waterShader.getUniformLocation("viewDistance");
	glUseProgram(0);

	return true;
}

void loadData() {
	// terrain data
	{
		terrainShader.use();

		int index = 0;
		for (int j = 0; j < gridHeight; j++) {
			for (int i = 0; i < gridWidth; i++) {
				vertices[index].position = vec3(i, 0.0f, j) - vec3(gridWidth / 2.0, 0.0, gridHeight / 2.0);
				index++;
			}
		}

		index = 0;
		for (int j = 0; j < gridHeight - 1; j++) {
			if ((j & 1) == 0) {
				for (int i = 0; i < gridWidth; i++) {
					indices[index++] = i + j * gridWidth;
					indices[index++] = i + (j + 1) * gridWidth;
				}
			}
			else {
				for (int i = gridWidth - 1; i > 0; i--) {
					indices[index++] = i + (j + 1) * gridWidth;
					indices[index++] = i - 1 + j * gridWidth;
				}
			}
		}

		glGenVertexArrays(1, &terrainVertexArrayObject);
		glBindVertexArray(terrainVertexArrayObject);

		GLuint vertexBufferObject;
		glGenBuffers(1, &vertexBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * numberOfVertices, vertices, GL_STATIC_DRAW);

		GLuint elementBufferObject;
		glGenBuffers(1, &elementBufferObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * numberOfIndices, indices, GL_STATIC_DRAW);

		size_t offset = 0;
		GLuint attrib;

		attrib = terrainShader.getAttribLocation("position");
		glEnableVertexAttribArray(attrib);
		glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (const GLvoid *) offset);
		offset += sizeof(vec3);

		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glUseProgram(0);
	}

	// water data
	{
		waterShader.use();

		glGenVertexArrays(1, &waterVertexArrayObject);
		glBindVertexArray(waterVertexArrayObject);

		static vertex vertices[4] = {
			vec3(         0.0f, 0.0f,           0.0f) - vec3(gridWidth / 2.0, 0.0, gridHeight / 2.0),
			vec3(gridWidth - 1, 0.0f,           0.0f) - vec3(gridWidth / 2.0, 0.0, gridHeight / 2.0),
			vec3(         0.0f, 0.0f, gridHeight - 1) - vec3(gridWidth / 2.0, 0.0, gridHeight / 2.0),
			vec3(gridWidth - 1, 0.0f, gridHeight - 1) - vec3(gridWidth / 2.0, 0.0, gridHeight / 2.0),
		};

		GLuint vertexBufferObject;
		glGenBuffers(1, &vertexBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * 4, vertices, GL_STATIC_DRAW);

		static GLuint indices[4] = {0, 2, 1, 3};

		GLuint elementBufferObject;
		glGenBuffers(1, &elementBufferObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 4, indices, GL_STATIC_DRAW);

		int offset = 0;
		GLuint attrib;

		attrib = waterShader.getAttribLocation("position");
		glEnableVertexAttribArray(attrib);
		glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (const GLvoid *) offset);
		offset += sizeof(vec3);

		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glUseProgram(0);
	}
}

void errorCallback(int error, const char* description) {
	std::cerr << "Error: " << description << std::endl;
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS) return;

	switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		case GLFW_KEY_SPACE:
			if (loadShaders()) {
				initialize();
				std::printf("shader reloaded\n");
			}
			break;
	}
}

void framebufferSizeCallback(GLFWwindow* window, int w, int h) {
	width = w;
	height = h;

	glViewport(0, 0, width, height);
	float ratio = width / (float) height;
	projectionMatrix = perspective(45.0f, ratio, 1.0f, 10000.0f);
}

void setup() {
	player.position = vec3(0.0f, 200.0f, 0.0f);
	player.direction = normalize(vec3(1.0, -1.0, 1.0));
	player.up = normalize(vec3(0.0f, 1.0f, 0.0f));
}

void createWindow() {
	GLFWmonitor *monitor = nullptr;

	if (fullscreen) {
		monitor = glfwGetPrimaryMonitor();

		int count;
		const GLFWvidmode *videoModes = glfwGetVideoModes(monitor, &count);

		width = videoModes[count - 1].width;
		height = videoModes[count - 1].height;
	} else {
		width = WINDOW_WIDTH;
		height = WINDOW_HEIGHT;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	window = glfwCreateWindow(width, height, "OpenGL", monitor, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, keyCallback);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		errorCallback(0, "GLEW could not be initialized");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// ignore potential GL_INVALID_ENUM error caused by glewInit
	GLenum error = glGetError();
	if (error != GL_NO_ERROR && error != GL_INVALID_ENUM) {
		errorPrint("after glewInit", error);
	}

	if (!loadShaders()) exit(EXIT_FAILURE);
	loadData();

	// update perspective transformation
	framebufferSizeCallback(window, width, height);

	glfwSwapInterval(1); // vsync on

	initialize();
}

void update(double time) {
	int keyTab = glfwGetKey(window, GLFW_KEY_TAB);
	if (keyTab && !keyTabPrev) {
		glfwDestroyWindow(window);

		fullscreen = !fullscreen;
		createWindow();

		return;
	}
	keyTabPrev = keyTab;

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	glfwSetCursorPos(window, 0, 0);

	float mouseSensitivity = 0.1f;
	float keySensitivity = 1.0f;

	vec3 delta = vec3(xpos, 0.0, ypos) * mouseSensitivity;

	if (glfwGetKey(window, GLFW_KEY_W)) {
		delta.p -= keySensitivity;
	}

	if (glfwGetKey(window, GLFW_KEY_S)) {
		delta.p += keySensitivity;
	}

	if (glfwGetKey(window, GLFW_KEY_Q)) {
		delta.y += keySensitivity;
	}

	if (glfwGetKey(window, GLFW_KEY_E)) {
		delta.y -= keySensitivity;
	}

	if (glfwGetKey(window, GLFW_KEY_A)) {
		delta.r -= keySensitivity;
	}

	if (glfwGetKey(window, GLFW_KEY_D)) {
		delta.r += keySensitivity;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
		player.speed += 0.01f;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
		player.speed -= 0.01f;
	}

	if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
		int count;
		const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);
		if (count >= 5) {
			float leftThumbX = axes[0];
			float leftThumbY = axes[1];
			float rightThumbX = axes[4];
			float rightThumbY = -axes[3];
			float triggers = -axes[2];

			static float thumbsDeadZone = 0.2f;
			static float triggersDeadZone = 0.001f;

			if (abs(leftThumbX)  < thumbsDeadZone)   leftThumbX  = 0.0f;
			if (abs(leftThumbY)  < thumbsDeadZone)   leftThumbY  = 0.0f;
			if (abs(rightThumbX) < thumbsDeadZone)   rightThumbX = 0.0f;
			if (abs(rightThumbY) < thumbsDeadZone)   rightThumbY = 0.0f;
			if (abs(triggers)    < triggersDeadZone) triggers    = 0.0f;

			delta.p = -leftThumbY * 2.0f;
			delta.r =  leftThumbX * 2.0f;
			player.speed = triggers * 3.0f;

			player.position.x += rightThumbX;
			player.position.z -= rightThumbY;
		}

		const unsigned char *buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
		if (count >= 6) {
			if (buttons[4]) {
				delta.y += 1.0f;
			}

			if (buttons[5]) {
				delta.y -= 1.0f;
			}
		}
	}

	player.position += player.direction * player.speed;

	vec3 right = cross(player.direction, player.up);
	vec3 forward = cross(player.up, right);

	player.direction = rotate(player.direction, delta.p, right);
	player.direction = rotate(player.direction, delta.y, player.up);
	player.up = rotate(player.up, delta.r, forward);
	player.up = rotate(player.up, delta.p, right);

	modelViewMatrix = lookAt(player.position, player.position + player.direction, player.up);
}

void initialize() {
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glClearColor(fogColor.r, fogColor.g, fogColor.b, 1.0);

	// upload uniforms
	terrainShader.use();
	glUniform3fv(terrainShaderUniform.fogColor, 1, value_ptr(fogColor));
	glUniform1f(terrainShaderUniform.viewDistance, viewDistance);
	glUseProgram(0);

	waterShader.use();
	glUniform3fv(waterShaderUniform.fogColor, 1, value_ptr(fogColor));
	glUniform1f(waterShaderUniform.viewDistance, viewDistance);
	glUseProgram(0);
}

void render(double time) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	terrainShader.use();

	// upload terrain shader uniforms
	glUniformMatrix4fv(terrainShaderUniform.projectionMatrix, 1, GL_FALSE, value_ptr(projectionMatrix));
	glUniformMatrix4fv(terrainShaderUniform.modelViewMatrix, 1, GL_FALSE, value_ptr(modelViewMatrix));
	glUniform1f(terrainShaderUniform.time, (float) time);
	glUniform3fv(terrainShaderUniform.playerPosition, 1, value_ptr(player.position));

	// draw terrain
	glBindVertexArray(terrainVertexArrayObject);
	glDrawElements(GL_TRIANGLE_STRIP, numberOfIndices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glUseProgram(0);

	waterShader.use();

	// upload water shader uniforms
	glUniformMatrix4fv(waterShaderUniform.projectionMatrix, 1, GL_FALSE, value_ptr(projectionMatrix));
	glUniformMatrix4fv(waterShaderUniform.modelViewMatrix, 1, GL_FALSE, value_ptr(modelViewMatrix));
	glUniform1f(waterShaderUniform.time, (float) time);
	glUniform3fv(waterShaderUniform.playerPosition, 1, value_ptr(player.position));

	// draw water
	glBindVertexArray(waterVertexArrayObject);
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glUseProgram(0);

	errorCheck("render exit");
}

int main(int argc, char *argv[]) {
	glfwSetErrorCallback(errorCallback);

	if (!glfwInit()) {
		return EXIT_FAILURE;
	}

	setup();
	createWindow();

	//double previousTime = 0;

	while (!glfwWindowShouldClose(window)) {
		double time = glfwGetTime();

		//double frametime = time - previousTime;
		//double fps = 1.0 / frametime;
		//std::printf("frametime: %.10fms, (fps: %.2f)\n", frametime * 1000.0, fps);
		//previousTime = time;

		update(time);
		render(time);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
}