#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include <cmath>

using namespace std;

struct Points{
    float x, y;
};

float rotationAngle = 0.0f;

float centerX = 30.0f;
float centerY = 30.0f;
float radius = 25.0f;

int numPoints = 8;

float PI = 3.14159265358979323846f;

vector<Points> coordinates;

vector<Points> getCoords(string filepath){

    ifstream file(filepath);
    string line;
    
    vector<Points>  coordinates;

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

void drawDog(float cx, float cy, float angle){
    
    glPushMatrix();

    // Moves the dog to the right place
    glTranslatef(cx, cy, 0);

    // rotates it
    glRotatef(angle, 0, 0, 1);

    glBegin(GL_LINE_STRIP);

    for (const auto &coord: coordinates){
        glVertex2f(coord.x, coord.y);
    }

    glEnd();

    glPopMatrix();

}

int main(void)
{        
    coordinates = getCoords("C:/Users/pikak/CS/Courses/CS3388/Assignment 2/dog.txt");

    if (!glfwInit() && !glewInit())
        return -1;
    
    GLFWwindow* window;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1280, 1000, "Dog", NULL, NULL);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // View Volume
    glOrtho(0, 60, 0, 60, -1.0, 1.0);
    
    // Background Color White
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Poll for and process events */
        glfwPollEvents();
        
		/* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // Line Color Black
        glColor3f(0.0f, 0.0f, 0.0f);

        // Draws the 8 dogs in the right 8 positions
        for (int i = 0; i < numPoints; i++){
            
            // Sets the angle the dog should be place on the circle
            float angle = i * 45.0f * PI / 180.0f;

            // Sets the x position on the circle
            float cx = centerX + radius * cos(angle);
            
            // Sets the y position on the circle
            float cy = centerY + radius * sin(angle);

            // Draws the dogs
            drawDog(cx, cy, rotationAngle);
        }

        // rotates the dog by 1 degree per frame
        rotationAngle += 1.0f;

        glFlush();

		/* Swap front and back buffers */
        glfwSwapBuffers(window);

    }

    glfwTerminate();
    return 0;
}
