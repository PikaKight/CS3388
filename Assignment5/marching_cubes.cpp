#include "marching_cubes.h"
#include "TriTable.hpp"
#include <glm/glm.hpp>
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


vector<float> marching_cubes(
    function<float(float, float, float)> f,
    float isovalue, float min, float max, float stepsize) {

    vector<float> vertices;

    for (float x = min; x < max; x += stepsize) {
        for (float y = min; y < max; y += stepsize) {
            for (float z = min; z < max; z += stepsize) {
                
                float cubeValues[8];
                vec3 corners[8] = {
                    {x, y, z}, {x+stepsize, y, z}, {x+stepsize, y+stepsize, z}, {x, y+stepsize, z},
                    {x, y, z+stepsize}, {x+stepsize, y, z+stepsize}, {x+stepsize, y+stepsize, z+stepsize}, {x, y+stepsize, z+stepsize}
                };

                for (int i = 0; i < 8; ++i)
                    cubeValues[i] = f(corners[i].x, corners[i].y, corners[i].z);

                int cubeIndex = 0;
                for (int i = 0; i < 8; ++i)
                    if (cubeValues[i] < isovalue) cubeIndex |= (1 << i);

                if (cubeIndex == 0 || cubeIndex == 255) continue;

                const int* edges = marching_cubes_lut[cubeIndex];  // Fix lookup table usage

                for (int i = 0; edges[i] != -1; i += 3) {
                    for (int j = 0; j < 3; ++j) {
                        int edge = edges[i + j];

                        int v1 = edge & 7;
                        int v2 = edge >> 3;

                        if (fabs(cubeValues[v2] - cubeValues[v1]) > 1e-6) { 
                            float t = (isovalue - cubeValues[v1]) / 
                                      (cubeValues[v2] - cubeValues[v1]);
                            vec3 p = corners[v1] + t * (corners[v2] - corners[v1]);
                        
                            vertices.push_back(p.x);
                            vertices.push_back(p.y);
                            vertices.push_back(p.z);
                        }
                    }
                }
            }
        }
    }

    return vertices;
}
