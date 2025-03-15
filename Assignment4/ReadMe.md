# Assignment 4 - Marcus

![](./Screenshot%201.png)
![](./Screenshot%202.png)

## Libraries

- GLEW
- GLUT
- GLM
- fstream
- filesystem
- standard (vector, cmath, string)

## Camera

The camera has 6 base variables:

```cpp
float camSpeed = 0.05f;
float camAngle = 3.0f;

vec3 camPos = vec3(0.5f, 0.4f, 0.5f);
vec3 camFront = vec3(0.0f, 0.0f, -1.0f);
vec3 camUp = vec3(0.0f, 1.0f, 0.0f);
```

These variables handle the camera movement speed, rotation angle, position, front of the camera, top of the camera.

Using the `processInput` function, I have 4 if statements to handle the up, down, left, right arrow keys.

The up and down keys handles going forward and backward by adding or subtracting from the camPos with the camFront \* camSpeed. This lets the camera move 0.05 units forward/backwards.

The left and right keys handles the rotation of the camera by calculating the new x and z values using the following:

left

```cpp
float angle = radians(camAngle);
float x = camFront.x * cos(angle) - camFront.z * sin(angle);
float z = camFront.x * sin(angle) + camFront.z * cos(angle);
camFront = normalize(vec3(x, 0.0f, z));
```

and

right

```cpp
float angle = radians(-camAngle);
float x = camFront.x * cos(angle) - camFront.z * sin(angle);
float z = camFront.x * sin(angle) + camFront.z * cos(angle);
camFront = normalize(vec3(x, 0.0f, z));
```

The only difference between the two is the direction which is denoted by whether camAngle is positive or negative.

## File Reading

The function `readPLYFile` is used to read the ply files to get the coordinates, normalized, and uv values.

The function reads the header line by line until the `end_header` line. Reading the header gives the number of vertices and faces, which is saved into the `vertexCount` and `faceCount` variables to determine how lone the for loops goes for.

Then 2 for loops are done to read rest of the lines and saves the values into the `vertices` and `faces` vectors.

The vertices for loop saves the coordinates, normalized coordinates and uv value into a temp `VertexData` and sets a default rgb value of white (1, 1, 1) since none of the ply has rgb. Then it is added to vertices.

The faces for loop saves the triangle vertices coordinates by reading the line, which contains:

- n -> number of vertices
- v1, v2, v3 -> vertices coordinates

The loop saves all four into temp variables and only saves v1, v2, v3 into faces.

## TexturedMesh

The class calls the `readPLYFile` function to get the vertices and faces. Then uses `loadTexture` and `loadShader` to set the texture and shaders. The function `setupMesh` sets up the textured mesh and the `draw` function draws it when its called.

`loadTexture` takes in the bmp file path and calls the `loadARGB_BMP` function from the `LoadBitmap` file. It gets the image data and sets the texture into `textureID`.

`loadShader` uses the vertices and faces to set the position and fragment color to set the shader.
