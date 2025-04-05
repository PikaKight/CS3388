
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
            gl_Position = MVP * vec4(pos, 1.0);
            
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
    

    // Compile Fragment Shader
    glShaderSource(fragmentShader, 1, &fragmentShaderSourcePointer, NULL);
    glCompileShader(fragmentShader);


    // Link Shaders into a Program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }

    // Clean up shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

float lastX = 400, lastY = 300;
bool firstMouse = true;
float zoomFactor = 0.5f;

// Variable to track whether the mouse button is pressed
bool isMousePressed = false;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // Check if the mouse button is pressed or released
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (!isMousePressed) {
            // Mouse button is pressed for the first time (on mouse down)
            isMousePressed = true;
            lastX = xpos; // reset last position when starting a drag
            lastY = ypos;
        }

        // Calculate the offset and move the camera only if the mouse is being dragged
        float xOffset = xpos - lastX;
        float yOffset = lastY - ypos;  // reversed because y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;

        camera.processMouse(xOffset, yOffset);
    }
    else {
        if (isMousePressed) {
            // Mouse button is released (on mouse up)
            isMousePressed = false;
        }
    }
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.processScroll(yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){

    if (key == GLFW_KEY_UP && action==GLFW_PRESS){
        camera.processKeyboard(zoomFactor);
    }

    if (key == GLFW_KEY_DOWN && action==GLFW_PRESS){
        camera.processKeyboard(-zoomFactor);
    }
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
    
    glLineWidth(1.0f);
    // Render the bounding box
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

}

void drawArrowHead(vec3 pos, vec3 direction, float size) {
    // Normalize direction
    vec3 arrowDirection = normalize(direction);

    // Choose an arbitrary perpendicular reference vector
    vec3 reference = (fabs(arrowDirection.y) > 0.9f) ? vec3(1.0f, 0.0f, 0.0f) : vec3(0.0f, 1.0f, 0.0f);

    // Compute perpendicular vectors
    vec3 right = normalize(cross(arrowDirection, reference));
    vec3 up = normalize(cross(right, arrowDirection));

    // Scale the arrowhead
    float arrowHeadLength = size * 0.2f;  // length of the arrowhead
    float arrowHeadWidth = size * 0.1f;   // width of the arrowhead

    // Define arrowhead vertices (triangle fan method)
    vec3 tip = pos + arrowDirection * arrowHeadLength;
    vec3 left = pos + right * arrowHeadWidth;
    vec3 rightV = pos - right * arrowHeadWidth;

    GLfloat arrowheadVertices[] = {
        tip.x, tip.y, tip.z,
        left.x, left.y, left.z,
        rightV.x, rightV.y, rightV.z
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(arrowheadVertices), arrowheadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Render the arrowhead (triangle fan)
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    // Cleanup
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void drawAxis(vec3 start, vec3 end) {
    
    GLfloat lineVertices[] = {
        start.x, start.y, start.z,
        end.x, end.y, end.z
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);

    // Vertex attribute pointer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glLineWidth(2.5f);
    // Render the line
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);

    vec3 direction = end - start;
    drawArrowHead(end, direction, 1.0f);
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
    glfwSetKeyCallback(window, key_callback);

    GLuint shaderID = loadShader();

    float isovalue = 0.0f, min = -1.5f, max =  1.5f, stepsize = 0.1f;

    auto vertices = marching_cubes([](float x, float y, float z) {
        return y - sin(x)*cos(z);
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
        glUniform3f(glGetUniformLocation(shaderID, "LightDir"), 0.0f, 0.0f, -1.0f);
        glUniform3f(glGetUniformLocation(shaderID, "modelColor"), 0.0f, 0.482f, 0.655f);
        glUniform1f(glGetUniformLocation(shaderID, "shininess"), 64.0f);
        glUniform3f(glGetUniformLocation(shaderID, "ambientColor"), 0.2f, 0.2f, 0.2f);
        glUniform3f(glGetUniformLocation(shaderID, "specularColor"), 1.0f, 1.0f, 1.0f);

        // Draw the bounding box and axes
        glUniform1i(glGetUniformLocation(shaderID, "hasOtherColor"), 1);
        glUniform3f(glGetUniformLocation(shaderID, "fragColor"), 1.0f, 1.0f, 1.0f);
        drawBoundaryBox(min, max);
        
        vec3 origin = vec3(min, min, min);
        vec3 xAxis = vec3((max-min), min, min);
        vec3 yAxis = vec3(min, (max-min), min);
        vec3 zAxis = vec3(min, min, (max-min));

        glUniform3f(glGetUniformLocation(shaderID, "fragColor"), 1.0f, 0.0f, 0.0f);
        drawAxis(origin, xAxis);

        glUniform3f(glGetUniformLocation(shaderID, "fragColor"), 0.0f, 1.0f, 0.0f);
        drawAxis(origin, yAxis);
        
        glUniform3f(glGetUniformLocation(shaderID, "fragColor"), 0.0f, 0.0f, 1.0f);
        drawAxis(origin, zAxis);


        glUniform1i(glGetUniformLocation(shaderID, "hasOtherColor"), 0);
        drawMarchingCubes(vertices, normals);

        glfwSwapBuffers(window);
    }

    
    glfwTerminate();
    return 0;
}

// g++ main.cpp camera.cpp marching_cubes.cpp -o main.exe -lfreeglut -lglew32 -lopengl32 -lglfw3 -lm -lstdc++