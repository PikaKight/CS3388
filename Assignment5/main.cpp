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

GLuint loadShader()
{

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    string vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 normal;

        uniform mat4 MVP;  // Model-view-projection matrix
        uniform mat4 V;    // View matrix
        uniform vec3 LightDir;  // Light direction

        out vec3 fragNormal;
        out vec3 fragLightDir;
        out vec3 fragViewDir;

        void main() {
            gl_Position = MVP * vec4(position, 1.0);
            
            fragNormal = normalize(mat3(V) * normal);  // Transform normal to view space
            fragLightDir = normalize(LightDir);  // Light direction in view space
            fragViewDir = normalize(-vec3(V * vec4(position, 1.0)));  // View direction
        }
        )";

    string fragmentShaderSource = R"(
        #version 330 core
        in vec3 fragNormal;
        in vec3 fragLightDir;
        in vec3 fragViewDir;

        uniform vec3 modelColor;  // Base color for diffuse component
        uniform vec3 ambientColor;  // Ambient light color
        uniform vec3 specularColor;  // Specular light color
        uniform float shininess;  // Shininess factor

        out vec4 FragColor;

        void main() {
            // Ambient component
            vec3 ambient = ambientColor * modelColor;

            // Diffuse component (Lambertian reflection)
            float diff = max(dot(fragNormal, fragLightDir), 0.0);
            vec3 diffuse = diff * modelColor;

            // Specular component (Phong reflection)
            vec3 reflectDir = reflect(-fragLightDir, fragNormal);
            float spec = pow(max(dot(fragViewDir, reflectDir), 0.0), shininess);
            vec3 specular = spec * specularColor;

            // Final color
            vec3 finalColor = ambient + diffuse + specular;
            FragColor = vec4(finalColor, 1.0);
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

float lastX = 400, lastY = 300;
bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos;  // reversed because y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    camera.processMouse(xOffset, yOffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.processScroll(yoffset);
}

void drawBoundaryBox(float min, float max){
    GLfloat vertices[] = {
        min, min, min,  min, min, max,  min, max, min,  min, max, max,  // front bottom-left, top-left
        max, min, min,  max, min, max,  max, max, min,  max, max, max,  // back bottom-right, top-right
        min, min, min,  max, min, min,  min, max, min,  max, max, min   // bottom-left to top-right edges
    };

    GLuint indices[] = {
        0, 1, 1, 3, 3, 2, 2, 0, 4, 5, 5, 7, 7, 4, 1, 5, 3, 7, 2, 6, 6, 0
    };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glBindVertexArray(0);

    // Render the bounding box
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void drawAxes(float min, float max) {
    GLfloat axesVertices[] = {
        min, min, min,  max, min, min,  // X-axis (Red)
        min, min, min,  min, max, min,  // Y-axis (Green)
        min, min, min,  min, min, max   // Z-axis (Blue)
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesVertices), axesVertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // Render the axes
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
}

void drawMarchingCubes(const vector<float>& vertices, const vector<float>& normals) {
    GLuint VBO, VAO, NBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &NBO);

    glBindVertexArray(VAO);

    // Store vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    // Store normals
    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // Draw the cubes
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);
    glBindVertexArray(0);
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

    glewInit();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    GLuint shaderID = loadShader();

    float isovalue = 0.0f, min = -1.5f, max =  1.5f, stepsize = 0.1f;

    auto vertices = marching_cubes([](float x, float y, float z) {
        return x*x + y*y + z*z - 1.0f;  // Sphere function
    }, 0.0f, -1.5f, 1.5f, 0.1f);

    vector<float> normals = compute_normals(vertices);

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glm::mat4 model = glm::mat4(1.0f);
        mat4 view = camera.getViewMatrix();
        mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
        mat4 MVP = projection * view * model;

        glUseProgram(shaderID);

        glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "V"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(shaderID, "LightDir"), 1.0f, 1.0f, -1.0f);
        glUniform3f(glGetUniformLocation(shaderID, "modelColor"), 0.8f, 0.2f, 0.2f);

        // Draw the bounding box and axes
        drawBoundaryBox(min, max);
        drawAxes(min, max);

        // Draw the marching cubes
        drawMarchingCubes(vertices, normals);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}
