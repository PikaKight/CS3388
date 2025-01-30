# Assignment 1

## Source Code

The code uses the libraries PyOpenGL and glfw to create a window and an Azure triangle with vertices (0,0.5),(0.5,−0.25),(−0.5,−0.25).

The code creates a window of 1280 px by 1000 px titled `Hello World` and opens the window.

While the window is opened, it checks for all pending events and then clear the active color. Then it sets the active color to Azure RGB(0, 50, 100).

Next it draws a Triangle with the given vertices to create an Azure triangle. Final it clears the buffers and repeats until the window is closed.
