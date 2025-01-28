#include "GLFW/glfw3.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include <cmath>

using namespace std;

vector<pair<float, float>> getCoords(){

    string filepath = "C:/Users/pikak/CS/Courses/CS3388/Assignment 2/dog.txt";


    ifstream file(filepath);
    string line;
    
    vector<pair<float, float>>  coordinates;

    if(!filesystem::exists(filepath)){
        cerr << "File doesn't exist\n";

        cout << "Current directory" << filesystem::current_path() << endl;

        return coordinates;
    }

    if (!file.is_open()){

        cerr << "Couldn't Open File\n";

        return coordinates;
    }

    while (getline(file, line)){
        stringstream ss(line);

        float x;
        float y;

        while(ss >> x >> y){

            coordinates.push_back({x, y});
        }
    }

    file.close();

    return (coordinates);
}

void CalculatePoints(float radius, float centerX, float centerY, int numPoints, float* points){

    // Calc 

}


void drawDog(const vector<pair<float, float>> &coords){
    glBegin(GL_LINE_STRIP);

    for (const auto &coord: coords){
        glVertex2f(coord.first, coord.second);
    }

    glEnd();

}

int main(void)
{
    int numPoints = 8;
    
    float radius = 25.0f;
    int centerX = 30;
    int centerY = 30;

    float rotationAngle = 0.0f;

    float points[2 * numPoints];
        
    vector<pair<float, float>> coordinates = getCoords();

    if (!glfwInit())
        return -1;
    
    GLFWwindow* window;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1280, 1000, "Hello World", NULL, NULL);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Poll for and process events */
        glfwPollEvents();
        
        // View Volume
        glOrtho(0, 60, 0, 60, -1.0, 1.0);

		/* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // Background Color White
        glClearColor(1.0f, 1.0f, 1.0f, 100);


        // Line Color Black
        glColor3f(0.0f, 0.0f, 0.0f);

        for (int i = 0; i < numPoints; i++){
        
            glPushMatrix();

            glTranslatef(points[2*i], points[2*i+1], 0.0f);

            // glRotated(rotationAngle, 0.0f, 0.0f, 1.0f);

            drawDog(coordinates);

            glPopMatrix();
        }

        rotationAngle += 1.0f;

        if (rotationAngle >= 360.0f){
            rotationAngle -= 360.0f;
        }
        
		/* Swap front and back buffers */
        glfwSwapBuffers(window);

    }

    glfwTerminate();
    return 0;
}
