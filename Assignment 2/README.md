# Assignment 2 - Marcus Tuen Muk

## Exercise 1

The code uses the Glew, GLFW, fstream, string, iostream, vector, filesystem and cmath library.

The code draws 8 dogs in a circle with a radius of 25 and center at (30,30). The dogs are consistently rotating at 1 degree per frame and are place at 0, 45, 90, 135, 180, 225, 270, 315 degrees on the circle.

**Struct Points:** A struct used to set the x and y value of a point

The function `getCoords` gets the points from the text file and places them into a vector of Points.

The function `drawDog` takes in the coordinate of where the dog would be placed on the circle and its rotation. It then draws the dog.

The main function sets the background to white and draws all 8 dogs. The dog's position is calculated by turning the angle (a multiple of 45) to radiants. Then using trigonometry to get the center x and y of the dog. Then the rotation angle increases by 1 degree every frame to produce the spinning effect.

### Run the Code

To run the code, please change the variable `path` to the path where the dog.txt is located before compiling and running the code.

## Exercise 2

The code uses the Glew, GLFW, fstream, string, iostream, vector, and cmath library.

The code also uses the `Stuct Points` to hold the coordinates of the points. The function `frand` gives a random float from -1 to 1.

`drawDotPot` draws N number of dots based on the pseudo code. The function sets the last corner to be a random corner on the square by using the rand mod 4 since there are 4 corners and the rand function will give a random number. A random point is chosen using the frand. The new corner is chosen using the same way, but will be rechosen if the corner is diagonal to the last corner. Finally, a new point is chosen that is halfway between the random point and the new corner. That point is drawn and the last corner is set to the new corner.

The number of points, screen width and height is set by the user using the arguments when running the code.
