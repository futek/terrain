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

GLuint shaderProgram;
GLuint positionAttrib;
GLuint projectionMatrixUniform, modelViewMatrixUniform, timeUniform, debugUniform, playerPositionUniform, fogColorUniform, viewDistanceUniform;
GLuint vertexArrayObject;

mat4 projectionMatrix, modelViewMatrix;

struct vertex {
	vec3 position;
};

const int gridWidth = 3000, gridHeight = 3000;
const GLuint numberOfVertices = gridWidth * gridHeight;
vertex vertices[numberOfVertices];
const GLuint numberOfIndicies = numberOfVertices + (gridWidth - 1) * (gridHeight - 2);
int indicies[numberOfIndicies];

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

GLuint setupUniform(const char* name) {
	GLuint location = glGetUniformLocation(shaderProgram, name);
	if (location == GL_INVALID_INDEX) {
		std::printf("shader did not contain '%s' uniform\n", name);
	}
	return location;
}

GLuint setupAttrib(const char* name) {
	GLuint location = glGetAttribLocation(shaderProgram, name);
	if (location == GL_INVALID_INDEX) {
		std::printf("shader did not contain '%s' attribute\n", name);
	}
	return location;
}

bool loadShaders() {
	GLuint program;

	bool success = createShaderProgram(&program, "shaders/terrain.vert", "shaders/terrain.frag", "outColor");
	if (!success) return false;

	shaderProgram = program;
	glUseProgram(shaderProgram);

	projectionMatrixUniform = setupUniform("projection");
	modelViewMatrixUniform = setupUniform("modelView");
	timeUniform = setupUniform("time");
	debugUniform = setupUniform("debug");
	playerPositionUniform = setupUniform("playerPosition");
	fogColorUniform = setupUniform("fogColor");
	viewDistanceUniform = setupUniform("viewDistance");

	positionAttrib = setupAttrib("position");

	return true;
}

void loadData() {
	{
		GLuint vertexBufferObject, elementBufferObject;

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
					indicies[index++] = i + j * gridWidth;
					indicies[index++] = i + (j + 1) * gridWidth;
				}
			}
			else {
				for (int i = gridWidth - 1; i > 0; i--) {
					indicies[index++] = i + (j + 1) * gridWidth;
					indicies[index++] = i - 1 + j * gridWidth;
				}
			}
		}

		glGenVertexArrays(1, &vertexArrayObject);
		glBindVertexArray(vertexArrayObject);

		glGenBuffers(1, &vertexBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * numberOfVertices, vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &elementBufferObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * numberOfIndicies, indicies, GL_STATIC_DRAW);

		size_t offset = 0;
		glEnableVertexAttribArray(positionAttrib);
		glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (const GLvoid *) offset);
		offset += sizeof(vec3);

		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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

	glClearColor(fogColor.r, fogColor.g, fogColor.b, 1.0);
	glUniform3fv(fogColorUniform, 1, value_ptr(fogColor));
	glUniform1f(viewDistanceUniform, viewDistance);
}

void render(double time) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, value_ptr(projectionMatrix));
	glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, value_ptr(modelViewMatrix));
	glUniform1f(timeUniform, (float) time);
	glUniform3fv(playerPositionUniform, 1, value_ptr(player.position));
	
	// draw terrain
	//glUseProgram(shaderProgram);
	glBindVertexArray(vertexArrayObject);
	glDrawElements(GL_TRIANGLE_STRIP, numberOfIndicies, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	//glUseProgram(0);

	errorCheck("render exit");
}

int main(int argc, char *argv[]) {
	glfwSetErrorCallback(errorCallback);

	if (!glfwInit()) {
		return EXIT_FAILURE;
	}

	setup();
	createWindow();

	while (!glfwWindowShouldClose(window)) {
		double time = glfwGetTime();

		update(time);
		render(time);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
}