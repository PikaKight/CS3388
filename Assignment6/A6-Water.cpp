#define GLM_ENABLE_EXPERIMENTAL

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

#include "PlaneMesh.hpp"

using namespace std;
using namespace glm;

GLFWwindow* window;

float cameraRadius = 10.0f;

void cameraControlsGlobe(mat4& ViewMatrix, float radius) {
    double xpos, ypos;
    static double lastX = 0.0, lastY = 0.0;
    static float azimuth = 0.0f, elevation = 0.0f;

    glfwGetCursorPos(window, &xpos, &ypos);
    float dx = xpos - lastX;
    float dy = ypos - lastY;
    lastX = xpos;
    lastY = ypos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        azimuth += dx * 0.005f;
        elevation += dy * 0.005f;
        if (elevation > radians(89.0f)) elevation = radians(89.0f);
        if (elevation < radians(10.0f)) elevation = radians(10.0f);
    }
	
    float x = radius * cos(elevation) * sin(azimuth);
    float y = radius * sin(elevation);
    float z = radius * cos(elevation) * cos(azimuth);

    vec3 eye = vec3(x, y, z);
    vec3 center = vec3(0, 0, 0);
    vec3 up = vec3(0, 1, 0);

    ViewMatrix = lookAt(eye, center, up);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    cameraRadius -= (float)yoffset * 0.5f;

    if (cameraRadius < 2.0f) cameraRadius = 2.0f;
    if (cameraRadius > 50.0f) cameraRadius = 50.0f;
}


GLuint LoadShaders(const char* vertex_file_path,
	const char* tess_control_path,
	const char* tess_eval_path,
	const char* geometry_file_path,
	const char* fragment_file_path) {

	auto CompileShader = [](const char* path, GLenum shaderType) -> GLuint {

		printf("Compiling %s\n", path);

		FILE* file = fopen(path, "rb");
		if (!file) { std::cerr << "Cannot open shader: " << path << "\n"; return 0; }

		fseek(file, 0, SEEK_END);
		long len = ftell(file);
		rewind(file);
		std::vector<char> src(len + 1);
		fread(&src[0], 1, len, file);
		fclose(file);
		src[len] = 0;

		GLuint shader = glCreateShader(shaderType);
		const char* ptr = &src[0];
		glShaderSource(shader, 1, &ptr, NULL);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char log[512];
			glGetShaderInfoLog(shader, 512, NULL, log);
			std::cerr << "Shader compilation error (" << path << "): " << log << "\n";
			return 0;
		}

		printf("Compiled %s\n\n", path);

		return shader;
	};

	GLuint vs = CompileShader(vertex_file_path, GL_VERTEX_SHADER);
	GLuint tcs = CompileShader(tess_control_path, GL_TESS_CONTROL_SHADER);
	GLuint tes = CompileShader(tess_eval_path, GL_TESS_EVALUATION_SHADER);
	GLuint gs = CompileShader(geometry_file_path, GL_GEOMETRY_SHADER);
	GLuint fs = CompileShader(fragment_file_path, GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, tcs);
	glAttachShader(program, tes);
	glAttachShader(program, gs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char log[1024];
		glGetProgramInfoLog(program, 1024, NULL, log);
		std::cerr << "Shader link error: " << log << "\n";
		return 0;
	}

	glDeleteShader(vs);
	glDeleteShader(tcs);
	glDeleteShader(tes);
	glDeleteShader(gs);
	glDeleteShader(fs);

	return program;
}

GLuint LoadBMPTexture(const char* imagepath) {
	printf("Loading texture: %s\n", imagepath);

	FILE* file = fopen(imagepath, "rb");
	if (!file) { std::cerr << "Image not found: " << imagepath << "\n"; return 0; }

	unsigned char header[54];
	fread(header, 1, 54, file);

	unsigned int dataPos = *(int*)&(header[0x0A]);
	unsigned int imageSize = *(int*)&(header[0x22]);
	unsigned int width = *(int*)&(header[0x12]);
	unsigned int height = *(int*)&(header[0x16]);

	if (imageSize == 0) imageSize = width * height * 3;
	if (dataPos == 0) dataPos = 54;

	std::vector<unsigned char> data(imageSize);
	fread(data.data(), 1, imageSize, file);
	fclose(file);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
				 0, GL_BGR, GL_UNSIGNED_BYTE, data.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	printf("texture Loaded: %s\n", imagepath);

	return textureID;
}

int main(int argc, char* argv[]) {
	float screenW = 1200, screenH = 720, stepsize = 1.0f;
	float xmin = -10, xmax = 10;

	if (argc > 1) screenW = atoi(argv[1]);
	if (argc > 2) screenH = atoi(argv[2]);
	if (argc > 3) stepsize = atof(argv[3]);
	if (argc > 4) xmin = atof(argv[4]);
	if (argc > 5) xmax = atof(argv[5]);

	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	window = glfwCreateWindow(screenW, screenH, "Phong", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		glfwTerminate();
		return -1;
	}

	GLuint shaderID = LoadShaders(
		"WaterShader.vertexshader",
		"WaterShader.tcs",
		"WaterShader.tes",
		"WaterShader.geoshader",
		"WaterShader.fragmentshader"
	);

	GLuint waterTexID = LoadBMPTexture("Assets/water.bmp");
	GLuint dispTexID = LoadBMPTexture("Assets/displacement-map1.bmp");

	PlaneMesh plane(xmin, xmax, stepsize, shaderID, waterTexID, dispTexID);

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetScrollCallback(window, scroll_callback);


	glClearColor(0.2f, 0.2f, 0.3f, 0.0f);
	
	glDisable(GL_CULL_FACE);

	mat4 Projection = perspective(radians(45.0f), screenW / screenH, 0.001f, 1000.0f);
	mat4 V;
	vec3 lightpos(5.0f, 30.0f, 5.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cameraControlsGlobe(V, cameraRadius);

		plane.draw(lightpos, V, Projection);

		glfwSwapBuffers(window);
		
		glfwPollEvents();
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window));

	glfwTerminate();
	return 0;
}
