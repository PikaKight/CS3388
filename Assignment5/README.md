# Assignment 5 - Marcus Tuen Muk

To use the code, run

``` bash
g++ main.cpp camera.cpp marching_cubes.cpp -o main.exe -lfreeglut -lglew32 -lopengl32 -lglfw3 -lm -lstdc++
./main.exe [WIDTH] [HEIGHT]
```

## Camera

The camera positions is calculated with the following

float x = r * sin(theta) * cos(phi);
    float y = r * sin(phi);
    float z = r * cos(theta) * cos(phi);

Where is mouse movement increases/decrease theta and phi.

The scroll wheel and arrow keys increases/decrease the zoom (r) by using the following
r = clamp(r - delta * 0.1f, 0.01f, 10.0f);

## Marching Cubes

The marching cubes uses the TriTable for the lookup.

The function use 3 loops for the x, y, z axis.
The corners using the vertTable and the step size to calculate the x, y, z values for the scalar functions.

The vertices are created by looping through the edges from the marching cube lookup tables and Interpolate vertex position with the vertTable.

## Writing to PLY

The header is manual written into the file. The vertices and normals are written by looping through.
