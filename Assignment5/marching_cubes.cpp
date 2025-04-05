#include "marching_cubes.h"
#include "TriTable.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace glm;

struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    // Vector subtraction
    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    // Cross product
    Vec3 cross(const Vec3& other) const {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    // Normalize the vector
    void normalize() {
        float length = sqrt(x * x + y * y + z * z);
        if (length > 0) {
            x /= length;
            y /= length;
            z /= length;
        }
    }
};

vector<float> compute_normals(const vector<float>& vertices) {
    vector<float> normals;
    int numVertices = vertices.size() / 3;
    normals.resize(vertices.size());

    for (int i = 0; i < numVertices; i += 3) {
        // Get three vertices of the triangle
        Vec3 v0(vertices[i], vertices[i + 1], vertices[i + 2]);
        Vec3 v1(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
        Vec3 v2(vertices[i + 6], vertices[i + 7], vertices[i + 8]);

        // Compute two edges
        Vec3 edge1 = v1 - v0;
        Vec3 edge2 = v2 - v0;

        // Cross product to get the normal
        Vec3 normal = edge1.cross(edge2);
        normal.normalize();  // Normalize the normal

        // Store the same normal for all three vertices of the triangle
        for (int j = 0; j < 3; ++j) {
            normals[i + j * 3] = normal.x;
            normals[i + j * 3 + 1] = normal.y;
            normals[i + j * 3 + 2] = normal.z;
        }
    }
    
    return normals;
}


// Function to interpolate a vertex along an edge using the vertTable
vec3 interpolateVertex(int edgeIndex, const vec3& p0, const vec3& p1, float val0, float val1, float isovalue) {
    // Compute the interpolation factor along the edge
    float t = (isovalue - val0) / (val1 - val0);
    t = std::clamp(t, 0.0f, 1.0f);

    // Compute the interpolated position using the edge direction from vertTable
    vec3 edgeDirection(vertTable[edgeIndex][0], vertTable[edgeIndex][1], vertTable[edgeIndex][2]);
    return vec3(
        p0.x + t * (p1.x - p0.x),
        p0.y + t * (p1.y - p0.y),
        p0.z + t * (p1.z - p0.z)
    );
}

vector<float> marching_cubes(
    function<float(float, float, float)> f,
    float isovalue, float min, float max, float stepsize) {

    vector<float> vertices;

    for (float z = min; z < max; z += stepsize) {
        for (float y = min; y < max; y += stepsize) {
            for (float x = min; x < max; x += stepsize) {

                float cubeValues[8];
                vec3 corners[8] = {
                    {x, y, z}, {x + stepsize, y, z}, {x + stepsize, y + stepsize, z}, {x, y + stepsize, z},
                    {x, y, z + stepsize}, {x + stepsize, y, z + stepsize}, {x + stepsize, y + stepsize, z + stepsize}, {x, y + stepsize, z + stepsize}
                };

                // Get scalar values at the 8 cube corners
                for (int i = 0; i < 8; i++) {
                    cubeValues[i] = f(corners[i].x, corners[i].y, corners[i].z);
                }

                int cubeIndex = 0;
                for (int i = 0; i < 8; ++i)
                    if (cubeValues[i] < isovalue) cubeIndex |= (1 << i);

                // If the cube is entirely inside or outside the isosurface, skip it
                if (cubeIndex == 0 || cubeIndex == 255) continue;

                // Get the edge table for the current cube configuration
                const int* edges = marching_cubes_lut[cubeIndex];

                // Iterate over the edges to form triangles
                for (int i = 0; edges[i] != -1; i += 3) {  // Each triangle has 3 edges
                    // Get the 3 edges that form the triangle
                    int edge1 = edges[i];
                    int edge2 = edges[i + 1];
                    int edge3 = edges[i + 2];

                    // Interpolate the vertices along the edges where the isosurface crosses
                    vec3 p1, p2, p3;

                    // Interpolate along edge1
                    int v0 = edge1 / 2;
                    int v1 = (edge1 + 1) / 2;
                    p1 = interpolateVertex(edge1, corners[v0], corners[v1], cubeValues[v0], cubeValues[v1], isovalue);

                    // Interpolate along edge2
                    v0 = edge2 / 2;
                    v1 = (edge2 + 1) / 2;
                    p2 = interpolateVertex(edge2, corners[v0], corners[v1], cubeValues[v0], cubeValues[v1], isovalue);

                    // Interpolate along edge3
                    v0 = edge3 / 2;
                    v1 = (edge3 + 1) / 2;
                    p3 = interpolateVertex(edge3, corners[v0], corners[v1], cubeValues[v0], cubeValues[v1], isovalue);

                    // Store the triangle vertices
                    vertices.push_back(p1.x);
                    vertices.push_back(p1.y);
                    vertices.push_back(p1.z);
                    vertices.push_back(p2.x);
                    vertices.push_back(p2.y);
                    vertices.push_back(p2.z);
                    vertices.push_back(p3.x);
                    vertices.push_back(p3.y);
                    vertices.push_back(p3.z);
                }
            }
        }
    }

    return vertices;
}
