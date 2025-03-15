#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

struct Point{

    float x, y;
};

struct Node: Point{

    bool hasHandle1, hasHandle2;

    Point handle1;
    Point handle2;
};

vector<Node> nodes;

Node* selectedNode = nullptr;
Point* selectedHandle = nullptr; 


int width;
int height;

// Control Point Distance and Tolerance
int cpDist = 50;
int pixelTolerance = 10;

// Number of Spline line ssegment
int N = 200;

/**
 * @brief Calculates the Bezier Curve point
 * 
 * @param p0 point 1
 * @param p1 point 1's handle 1
 * @param p2 point 2's handle 1
 * @param p3 point 2
 * @param t  interval
 * @return Point the bezier curve point
 */
Point calc_bezier(Point p0, Point p1, Point p2, Point p3, float t){

    float u = (1-t);
    float u2 = pow(u, 2);
    float u3 = pow(u, 3);

    float t2 = pow(t, 2);
    float t3 = pow(t, 3);

    float bezierX = (u3 * p0.x) + (u2 * t* p1.x * 3) + (u * t2 * p2.x * 3) + (t3 * p3.x);
    float bezierY = (u3 * p0.y) + (u2 * t* p1.y * 3) + (u * t2 * p2.y * 3) + (t3 * p3.y);

    return {bezierX, bezierY};
}

/**
 * @brief Calculate distance between 2 points
 * 
 * @param p1 point 1
 * @param p2 point 2
 * @return float distance between 2 points
 */
float calc_dist(Point p1, Point p2){
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;

    float dist = sqrt(pow(dx, 2) + pow(dy, 2));

    return dist;
}

/**
 * @brief Draws the spline
 * 
 */
void draw_Spline(){

    // Only draws the spline if there are 2 endpoints
    if (nodes.size() < 2) return;

    // Sets Line color to black and 2px width
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(2);

    // Enables the line to be smooth
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Draws it as a single line
    glBegin(GL_LINE_STRIP);
    
    // goes through n-1 nodes
    for (size_t i = 0; i < nodes.size() - 1; i++){
        Node &p0 = nodes[i], &p1 = nodes[i+1];
        
        // draws it based on the bezier curve points
        for (float t = 0.0; t <= 1.0; t+=(1.0/N)){
            Point p = calc_bezier(p0, p0.handle1, p1.handle1, p1, t);
            glVertex2f(p.x, p.y);
        }
    }

    glEnd();
    glDisable(GL_LINE_SMOOTH);
}

/**
 * @brief Draws the nodes
 * 
 */
void draw_Point(){

    glPointSize(10);
    glBegin(GL_POINTS);

    for (auto &node: nodes){
        glColor3f(0.0f, 0.0f, 1.0f);

        glVertex2f(node.x, node.y);

    }

    glEnd();
}

/**
 * @brief Draws the control points
 * 
 */
void draw_Control_Point(){

    // Enables the point to be circular
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPointSize(10);
    glBegin(GL_POINTS);

    glColor3f(0.0f, 0.0f, 0.0f);

    // Loops through each node to get the control points
    for (auto &node: nodes){
        
        // By default each node has at least 1 control point
        glVertex2f(node.handle1.x, node.handle1.y);

        // Checks if it has a second control points and draws it
        if (node.hasHandle2) glVertex2f(node.handle2.x, node.handle2.y);

    }

    glEnd();

    // Disables the smooth dot so the node can be square
    glDisable(GL_POINT_SMOOTH);

}

/**
 * @brief Draws the dotted line to show which control point goes to which node
 * 
 */
void draw_Connections(){
    glColor3f(0.0, 1.0, 0.0);
    glLineWidth(2);

    // Enables the dotted line
    glEnable(GL_LINE_STIPPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineStipple(1, 0x00FF);

    glBegin(GL_LINES);
    
    for (auto &node: nodes){
        
        // Draws the line to the first control point
        glVertex2f(node.x, node.y);
        glVertex2f(node.handle1.x, node.handle1.y);
        
        // Draws the line to the second control point if it exists
        if (node.hasHandle2){
            glVertex2f(node.x, node.y);
            glVertex2f(node.handle2.x, node.handle2.y);
        }

    }

    glEnd();

    // Disables dotted line
    glDisable(GL_LINE_STIPPLE);
}

/**
 * @brief Adds a new node
 * 
 * @param cx X coordinate of the node
 * @param cy Y Coordinate of the node
 */
void add_Node(float cx, float cy){

    // Sets the new node's settings
    Node newNode;

    newNode.x = cx;
    newNode.y = cy;

    newNode.handle1.x = cx;
    newNode.handle1.y = cy + cpDist;

    newNode.hasHandle1 = true;
    newNode.hasHandle2 = false;

    // Checks if the new node is the first 2 endpoints
    if (nodes.size() < 2){
        nodes.push_back(newNode);
        return;
    }

    // Gets the 2 endpoint
    Node &startNode = nodes.front();
    Node &endNode = nodes.back();

    // Calculates the distance between the new node and the endpoints
    float distStart = calc_dist(newNode, startNode);
    float distEnd = calc_dist(newNode, endNode);

    // See which endpoint is closer
    if (distStart < distEnd){
        
        // Adds a second control point to the starting endpoint
        startNode.hasHandle2 = true;

        // Sets the coordinates to be equal distant and in a line to the first control point
        startNode.handle2.x = startNode.x - (startNode.handle1.x - startNode.x);
        startNode.handle2.y = startNode.y - (startNode.handle1.y - startNode.y);

        // Sets the new node at the front so that its the new starting endpoint
        nodes.insert(nodes.begin(), newNode);
    }
    else if (distStart > distEnd){
    
        // Adds a second control point to the ending endpoint
        endNode.hasHandle2 = true;

        // Sets the coordinates to be equal distant and in a line to the first control point
        endNode.handle2.x = endNode.x - (endNode.handle1.x - endNode.x);
        endNode.handle2.y = endNode.y - (endNode.handle1.y - endNode.y);

        // Sets the new node as the new ending endpoint
        nodes.push_back(newNode);
    }
}

/**
 * @brief Controls the mouse interaction
 */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Handles left click
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        
        // Gets the cursor position and sets it
        double cx, cy;
        glfwGetCursorPos(window, &cx, &cy);

        // Converts y coordinate to bottom left origin
        cy = height - cy;

        // Loops through all the nodes to check if a node or control point was clicked
        for (auto &node: nodes){
            
            // Sets the selected node or control point if within tolerance
            if (hypot(cx - node.handle1.x, cy - node.handle1.y) < pixelTolerance){
                selectedHandle = &node.handle1;
                return; 
            }
            else if (node.hasHandle2 && hypot(cx - node.handle2.x, cy - node.handle2.y) < pixelTolerance){
                selectedHandle = &node.handle2;
                return; 
            }
            else if (hypot(cx - node.x, cy - node.y) < pixelTolerance){
                selectedNode = &node;
                return; 
            }
        }

        // Adds new node if clicked on an empty space
        add_Node((float) cx, (float) cy);
    }

    // Resets selected node and control point when left mouse button is released 
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
        selectedNode = nullptr;
        selectedHandle = nullptr;
    }
}

/**
 * @brief Handles the cursor interactions
 */
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    // Gets the cursor position and converts to bottom left origin
    glfwGetCursorPos(window, &xpos, &ypos);
    ypos = height - ypos;

    // Checks if the node is selected
    if (selectedNode){

        // Calculates the move distance
        float dx = xpos - selectedNode->x;
        float dy = ypos - selectedNode->y;

        // Moves the node
        selectedNode -> x = xpos;
        selectedNode -> y = ypos;

        // Moves the control points by the distance
        selectedNode->handle1.x += dx;
        selectedNode->handle1.y += dy;
        
        if (selectedNode->hasHandle2){
            selectedNode->handle2.x += dx;
            selectedNode->handle2.y += dy;
        }
    }

    // Checks if a control point is selected
    else if (selectedHandle){
        
        // Moves the control point
        selectedHandle->x = xpos;
        selectedHandle->y = ypos;

        // Loops through to find the corrsponding node and move the other control point if it exists
        for (auto &node: nodes){
            if (selectedHandle == &node.handle1 && node.hasHandle2){
                node.handle2.x = node.x - (node.handle1.x - node.x);
                node.handle2.y = node.y - (node.handle1.y - node.y);
            }
            if (selectedHandle == &node.handle2 && node.hasHandle1){
                node.handle1.x = node.x - (node.handle2.x - node.x);
                node.handle1.y = node.y - (node.handle2.y - node.y);
            }
        }
    }
}

/**
 * @brief Handles the keyboard interactions
 */
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    
    // Clears the window if 'e' is pressed
    if (key == GLFW_KEY_E && action == GLFW_PRESS){
        nodes.clear();
    }
}

int main(int argc, char** argv)
{        
    // Throws error if the width and height aren't given
    if (argc != 3){
        cerr << "Please Enter the Screen Width and Screen Height\n";
        return -1;
    }

    // Sets width and height
    width = atoi(argv[1]); 
    height = atoi(argv[2]);

    if (!glfwInit() && !glewInit())
        return -1;
    
    GLFWwindow* window;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, "Spline Tool", NULL, NULL);

    glfwMakeContextCurrent(window);

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    // Mouse Callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);

    // View Volume and viewport
    glOrtho(0, width, 0, height, -1.0, 1.0);
    glViewport(0, 0, width, height);

    // Background Color White
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glEnable(GL_MULTISAMPLE);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Poll for and process events */
        glfwPollEvents();
        
		/* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        draw_Spline();
        draw_Connections();
        draw_Point();
        draw_Control_Point();
        
        glFlush();

		/* Swap front and back buffers */
        glfwSwapBuffers(window);

    }

    glfwTerminate();
    return 0;
}
