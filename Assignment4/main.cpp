#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include <cmath>
#include <bits/stdc++.h>

#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "LoadBitmap.h"

using namespace std;
using namespace glm;
using namespace filesystem;

float camSpeed = 0.05f;
float camAngle = 3.0f;

vec3 camPos = vec3(0.5f, 0.4f, 0.5f);
vec3 camFront = vec3(0.0f, 0.0f, -1.0f);
vec3 camUp = vec3(0.0f, 1.0f, 0.0f);

string PATH = "./LinksHouse";

struct VertexData
{
    float x, y, z;
    float nx, ny, nz;
    float r, g, b;
    float u, v;
};

struct TriData
{
    int indices[3];
};

void readPLYFile(string fname, vector<VertexData> &vertices, vector<TriData> &faces)
{
    ifstream file(fname);
    string line;
    int vertexCount = 0;
    int faceCount = 0;

    while (getline(file, line))
    {
        if (line.rfind("element vertex", 0) == 0)
        {
            vertexCount = stoi(line.substr(15));
        }
        else if (line.rfind("element face", 0) == 0)
        {
            faceCount = stoi(line.substr(13));
        }
        else if (line == "end_header")
        {
            break;
        }
    }

    for (int i = 0; i < vertexCount; i++)
    {
        VertexData v = {};

        file >> v.x >> v.y >> v.z >> v.nx >> v.ny >> v.nz >> v.u >> v.v;

        v.r = 1.0f;  // White color by default
        v.g = 1.0f;
        v.b = 1.0f;

        vertices.push_back(v);
    }

    for (int i = 0; i < faceCount; i++)
    {
        int n, v1, v2, v3;
        file >> n >> v1 >> v2 >> v3;
        faces.push_back({v1, v2, v3});
    }

}

GLuint loadTexture(const string &fname)
{
    unsigned char *imgData;
    unsigned int width, height;
    loadARGB_BMP(fname.c_str(), &imgData, &width, &height);

    GLuint textureID;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, imgData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete[] imgData;
    return textureID;
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

    // Compile Fragment Shader
    glShaderSource(fragmentShader, 1, &fragmentShaderSourcePointer, NULL);
    glCompileShader(fragmentShader);

    // Link Shaders into a Program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Clean up shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

class TexturedMesh {
    public:
        GLuint VBO, VAO, EBO, textureID, shaderProgram;
        vector<VertexData> vertices;
        vector<TriData> faces;
    
        TexturedMesh(const string& plyFile, const string& bmpFile) {

            readPLYFile(plyFile, vertices, faces);
            textureID = loadTexture(bmpFile);
            shaderProgram = loadShader();
            setupMesh();
        }
    
        void setupMesh() {

            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);
    
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData), &vertices[0], GL_STATIC_DRAW);
    
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(TriData), &faces[0], GL_STATIC_DRAW);
    
            GLsizei stride = sizeof(VertexData);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
            glEnableVertexAttribArray(0);
    
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
    
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);
    
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
            glEnableVertexAttribArray(3);
    
            glBindVertexArray(0);
        }
    
        void draw(glm::mat4 MVP) {
            
            glUseProgram(shaderProgram);

            GLuint MVP_Location = glGetUniformLocation(shaderProgram, "MVP");
            glUniformMatrix4fv(MVP_Location, 1, GL_FALSE, glm::value_ptr(MVP));
    
            glBindVertexArray(VAO);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    };

    void processInput(GLFWwindow* window) {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            camPos += camSpeed * camFront;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            camPos -= camSpeed * camFront;
    
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            float angle = radians(camAngle);
            float x = camFront.x * cos(angle) - camFront.z * sin(angle);
            float z = camFront.x * sin(angle) + camFront.z * cos(angle);
            camFront = normalize(vec3(x, 0.0f, z));
        }
    
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            float angle = radians(-camAngle);
            float x = camFront.x * cos(angle) - camFront.z * sin(angle);
            float z = camFront.x * sin(angle) + camFront.z * cos(angle);
            camFront = normalize(vec3(x, 0.0f, z));
        }
    }

    vector<TexturedMesh> opaque;
    vector<TexturedMesh> trans;

    void setMesh() {
        vector<string> bmp;
        vector<string> ply;
    
        vector<string> transObjs = {"curtains.bmp", "Curtains.ply", "doorbg.bmp", "DoorBG.ply", "metalobjects.bmp", "MetalObjects.ply"};

        for (const auto& file : directory_iterator(PATH)) {
            if (!file.is_regular_file()) continue;
    
            if (file.path().extension() == ".bmp")
                bmp.push_back(file.path().string());
            if (file.path().extension() == ".ply")
                ply.push_back(file.path().string());
        }
    
        size_t numFiles = bmp.size();
        for (size_t i = 0; i < numFiles; i++){

            bool isTransPLY = find(transObjs.begin(), transObjs.end(), path(ply[i]).filename().string()) != transObjs.end();
            bool isTransBMP = find(transObjs.begin(), transObjs.end(), path(bmp[i]).filename().string()) != transObjs.end();

            if (isTransPLY && isTransBMP){
                trans.push_back(TexturedMesh(ply[i], bmp[i]));
                continue;
            }

            opaque.push_back(TexturedMesh(ply[i], bmp[i]));
            
        }
    }

    int main(int argc, char** argv) {
        
        if (argc != 3){
            cerr << "Please Enter the Screen Width and Screen Height\n";
            return -1;
        }

        int width = atoi(argv[1]); 
        int height = atoi(argv[2]);
    
        if (!glfwInit()) return -1;
        GLFWwindow* window = glfwCreateWindow(width, height, "PLY Renderer", NULL, NULL);
        glfwMakeContextCurrent(window);
        
        glewInit();
        glEnable(GL_DEPTH_TEST);
    
        mat4 projection = perspective(radians(45.0f), float(width) / float(height), 0.1f, 100.0f);
    
        setMesh();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            processInput(window);
    
            mat4 model = mat4(1.0f);
            model = translate(model, vec3(0.0f, 0.0f, -1.0f));  // Translate the object along the Z-axis

            mat4 view = lookAt(camPos, camPos + camFront, camUp);
            mat4 MVP = projection * view * model;

            for (auto& mesh : opaque) {
                mesh.draw(MVP);
            }
    
            glDepthMask(GL_FALSE);  

            for (auto& mesh : trans) {
                mesh.draw(MVP);
            }
            
            glDepthMask(GL_TRUE);

            glfwSwapBuffers(window);
        }
    
        glfwTerminate();
        return 0;
    }
