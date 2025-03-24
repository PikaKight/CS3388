#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <cmath>
#include <bits/stdc++.h>
#include <filesystem>

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
        layout(location = 0) in vec3 pos;
        layout(location = 1) in vec3 normal;

        uniform mat4 MVP;  // Model-view-projection matrix
        uniform mat4 V;    // View matrix
        uniform vec3 LightDir;  // Light direction

        out vec3 fragNormal;
        out vec3 fragLightDir;
        out vec3 fragViewDir;

        void main() {
            gl_pos = MVP * vec4(pos, 1.0);
            
            fragNormal =normalize(mat3(transpose(inverse(V))) * normal); // Transform normal to view space
            fragLightDir = normalize(LightDir);  // Light direction in view space
            fragViewDir = normalize(-vec3(V * vec4(pos, 1.0)));  // View direction
        }
        )";

        string fragmentShaderSource = R"(
            #version 330 core
            in vec3 fragNormal;
            in vec3 fragLightDir;
            in vec3 fragViewDir;
        
            uniform vec3 modelColor;  // Base color for mesh
            uniform vec3 fragColor;   // Custom Color
            uniform vec3 ambientColor;  // Ambient light color
            uniform vec3 specularColor;  // Specular light color
            uniform float shininess;  // Shininess factor
            uniform bool hasOtherColor;

            out vec4 FragColor;
        
            void main() {        
                if (hasOtherColor){
                    FragColor = vec4(fragColor, 1.0);
                    return;
                }

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
        // Front Face
        min, min, min, // Front Bottom Left
        max, min, min, // Front Bottom Right
        max, max, min, // Front Top Right
        min, max, min, // Front Top Left

        // Back Face
        min, min, max, // Back Bottom Left
        max, min, max, // Back Bottom Right
        max, max, max, // Back Top Right
        min, max, max, // Back Top Left
    };

    GLuint indices[] = {
        0, 1, 1, 2, 2, 3, 3, 0, // Front face
        4, 5, 5, 6, 6, 7, 7, 4, // Back face
        0, 4, 1, 5, 2, 6, 3, 7  // Connecting edges
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

    // pos attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glBindVertexArray(0);

    // Render the bounding box
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

}
/**
void drawArrowHead(vec3 pos, vec3 direction, float size){
    vec3 right = normalize(cross(direction, vec3(0.0f, 0.0f, 1.0f))) * size;
    vec3 left = normalize(cross(direction, vec3(0.0f, 0.0f, -1.0f))) * size;

    GLfloat vertices[] = {
        pos.x, pos.y, pos.z,  // Base of the triangle
        pos.x + direction.x * size, pos.y + direction.y * size, pos.z + direction.z * size, // Tip of the arrowhead
        pos.x + right.x, pos.y + right.y, pos.z + right.z,  // Right side of the triangle
        pos.x + left.x, pos.y + left.y, pos.z + left.z   // Left side of the triangle
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Enable vertex attributes (position)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Draw the arrowhead (as a triangle fan)
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Clean up
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawAxis(vec3 start, vec3 direction, float max) {
    
    vec3 end = direction * max;

    GLfloat axisvertices[] = {
        start.x, start.y, start.z,
        end.x, end.y, end.z
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisvertices), axisvertices, GL_STATIC_DRAW);

    // pos attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // Render the axes
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 3);
    glBindVertexArray(0);
}
*/
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // Draw the cubes
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &NBO);

}

void writePLY(const vector<float>& vertices, const vector<float>& normals, const string& fileName) {
    ofstream outFile(fileName);

    if (!outFile) {
        cerr << "Error opening file: " << fileName << endl;
        return;
    }

    // Write PLY header
    outFile << "ply\n\n";
    outFile << "format ascii 1.0\n";
    outFile << "element vertex " << vertices.size() / 3 << "\n";
    outFile << "property float x\n";
    outFile << "property float y\n";
    outFile << "property float z\n";
    outFile << "property float nx\n";
    outFile << "property float ny\n";
    outFile << "property float nz\n";
    outFile << "element face " << vertices.size() / 3 / 3 << "\n";
    outFile << "property list uchar int vertex_index\n";
    outFile << "end_header\n\n";

    // Write vertices and normals
    for (size_t i = 0; i < vertices.size() / 3; ++i) {
        outFile << vertices[i * 3] << " " << vertices[i * 3 + 1] << " " << vertices[i * 3 + 2] << " ";
        outFile << normals[i * 3] << " " << normals[i * 3 + 1] << " " << normals[i * 3 + 2] << "\n";
    }

    // Write faces (triangles)
    for (size_t i = 0; i < vertices.size() / 3 / 3; ++i) {
        outFile << "3 " << i * 3 << " " << i * 3 + 1 << " " << i * 3 + 2 << "\n";
    }

    outFile.close();
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

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    GLuint shaderID = loadShader();

    float isovalue = 0.0f, min = -1.5f, max =  1.5f, stepsize = 0.1f;

    auto vertices = marching_cubes([](float x, float y, float z) {
        return x*x + y*y + z*z - 1.0f;  // Sphere function
    }, isovalue, min, max, stepsize);

    vector<float> normals = compute_normals(vertices);

    writePLY(vertices, normals, "./fileName.ply");

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
        glUniform3f(glGetUniformLocation(shaderID, "modelColor"), 0.0f, 0.482f, 0.655f);
        glUniform1f(glGetUniformLocation(shaderID, "shininess"), 64.0f);
        glUniform3f(glGetUniformLocation(shaderID, "ambientColor"), 0.2f, 0.2f, 0.2f);
        glUniform3f(glGetUniformLocation(shaderID, "specularColor"), 1.0f, 1.0f, 1.0f);

        
        // Draw the bounding box and axes
        glUniform1i(glGetUniformLocation(shaderID, "hasOtherColor"), true);
        glUniform3f(glGetUniformLocation(shaderID, "fragColor"), 1.0f, 1.0f, 1.0f);
        drawBoundaryBox(min, max);
/**
        vec3 origin = vec3(min, min, max);
        vec3 xAxis = vec3(min, min, max);
        vec3 yAxis = vec3(min, min, max);
        vec3 zAxis = vec3(min, min, max);

        glUniform3f(glGetUniformLocation(shaderID, "fragColor"), 1.0f, 0.0f, 0.0f);
        drawAxis(origin, xAxis, max);
        drawArrowHead(origin, xAxis, 2.0f);

        glUniform3f(glGetUniformLocation(shaderID, "fragColor"), 0.0f, 1.0f, 0.0f);
        drawAxis(origin, yAxis, max);
        drawArrowHead(origin, yAxis, 2.0f);
        
        glUniform3f(glGetUniformLocation(shaderID, "fragColor"), 0.0f, 0.0f, 1.0f);
        drawAxis(origin, zAxis, max);
        drawArrowHead(origin, zAxis, 2.0f);
*/

        glUniform1i(glGetUniformLocation(shaderID, "hasOtherColor"), false);
        drawMarchingCubes(vertices, normals);

        glfwSwapBuffers(window);
    }

    
    glfwTerminate();
    return 0;
}
