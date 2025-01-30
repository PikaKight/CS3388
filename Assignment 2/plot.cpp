#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include <cmath>

#define N_MAX 5000000

using namespace std;

struct Points{
    double x, y;
};

double xMin= -1.0, yMin = -1.0;
double xMax= 1.0, yMax = 1.0;

double frand(){
    double x = ((double)rand())/(double) RAND_MAX;
 
    return ((rand() % 2 ) ? -x:x);
}

void drawDotPlot(int N){
    Points corners[4] = {
        {xMin, yMin}, // Bottom left
        {xMin, yMax}, // Top Left
        {xMax, yMax}, // Top Right
        {xMax, yMin} // Bottom Right
        }; 

    // Chooses a random point within [(-1,-1), (1, 1)]
    Points pt = {frand(), frand()};
    
    // Choose a random corner
    int lastCorner = rand() % 4;
    
    // Point Color Black
    glColor3f(0.0f, 0.0f, 0.0f);

    // Point Size 2.0
    glPointSize(2.0f);

    glBegin(GL_POINTS);

    // Draws N number of dots
    for (int i = 0; i < N; i++){
        int newCorner;

        do{
            newCorner = rand() % 4;
        } while (
            (lastCorner + 2) % 4 == newCorner
        );

        // setting p_i to be halfway b/w p_i-1 and the new corner
        pt.x = (pt.x + corners[newCorner].x) / 2;
        pt.y = (pt.y + corners[newCorner].y) / 2;

        glVertex2f(pt.x, pt.y);

        lastCorner = newCorner;
    }

    glEnd();

    glFlush();
    
}

int main(int argc, char** argv)
{        
    srand(time(0));

    if (argc != 4){
        cout << "Please Enter N, Screen Width, Screen Height\n";
        return -1;
    }

    int N = min(atoi(argv[1]), N_MAX);

    int width = atoi(argv[2]);
    int height = atoi(argv[3]);

    if (!glfwInit())
        return -1;
    
    GLFWwindow* window = glfwCreateWindow(width, height, "Dot Plot", NULL, NULL);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // View Volume
    glOrtho(-1.1, 1.1, -1.1, 1.1, -1.0, 1.0);
    
    // Background Color White
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
	/* Render here */
    glClear(GL_COLOR_BUFFER_BIT);

    // Draws the dot plot
    drawDotPlot(N);

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
