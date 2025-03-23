#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <cmath>
#include <bits/stdc++.h>

#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "marching_cubes.h"
#include "camera.h"

using namespace std;
using namespace glm;

Camera camera;

void framebuffer_size_callback(GLFWwindow *window, int w, int h)
{
    glViewport(0, 0, w, h);
}

GLuint loadShader()
{

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    string vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 3) in vec2 aTexCoord;
        
        uniform mat4 MVP;
        
        out vec2 TexCoord;
        
        void main() {
            gl_Position = MVP * vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
        )";

    string fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec2 TexCoord;
        uniform sampler2D texture1;
        
        void main() {

            FragColor = texture(texture1, TexCoord);
        }
        )";

    // Compile Vertex Shader
    const char *vertexShaderSourcePointer = vertexShaderSource.c_str();
    const char *fragmentShaderSourcePointer = fragmentShaderSource.c_str();

    glShaderSource(vertexShader, 1, &vertexShaderSourcePointer, NULL);
    glCompileShader(vertexShader);

    // Check Vertex Shader Compilation
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    

    // Compile Fragment Shader
    glShaderSource(fragmentShader, 1, &fragmentShaderSourcePointer, NULL);
    glCompileShader(fragmentShader);

    // Check Fragment Shader Compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    

    // Link Shaders into a Program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check Program Linking
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
   

    // Clean up shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main(int argc, char **argv)
{

    if (argc != 3)
    {
        cerr << "Please Enter the Screen Width and Screen Height\n";
        return -1;
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);

    if (!glfwInit())
        return -1;

    GLFWwindow *window = glfwCreateWindow(width, height, "Marching Cubes", NULL, NULL);

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewInit();
    glEnable(GL_DEPTH_TEST);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);


    GLuint shaderID = loadShader();

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        mat4 view = camera.getViewMatrix();
        mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
        mat4 MVP = projection * view;

        glUseProgram(shaderID);
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), 1, GL_FALSE, value_ptr(MVP));
        glUniform3f(glGetUniformLocation(shaderID, "modelColor"), 1.0f, 0.5f, 0.3f);

        
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}
