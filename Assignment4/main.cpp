#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include <cmath>

#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "LoadBitmap.cpp"

using namespace std;
using namespace glm;

float camSpeed = 0.05f;
float camAngle = 3.0f;

vec3 camPos = vec3(0.5f, 0.4f, 0.5f);
vec3 camFront = vec3(0.0f, 0.0f, -1.0f);
vec3 camUp = vec3(0.0f, 1.0f, 0.0f);

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camPos += camSpeed * camPos;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camPos -= camSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        float x = camFront.x * cos(camAngle) - camFront.z * sin(camAngle);
        float z = camFront.x * sin(camAngle) - camFront.z * cos(camAngle);
        camFront = normalize(vec3(x, 0.0f, z));
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        float x = camFront.x * cos(-camAngle) - camFront.z * sin(-camAngle);
        float z = camFront.x * sin(-camAngle) - camFront.z * cos(-camAngle);
        camFront = normalize(vec3(x, 0.0f, z));
    }
}

int main(int argc, char **argv)
{
    srand(time(0));

    if (argc != 3)
    {
        cout << "Please Enter Screen Width, Screen Height\n";
        return -1;
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);

    if (!glfwInit())
        return -1;

    GLFWwindow *window = glfwCreateWindow(width, height, "Dot Plot", NULL, NULL);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // View Volume
    glOrtho(-1.1, 1.1, -1.1, 1.1, -1.0, 1.0);

    // Background Color White
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
