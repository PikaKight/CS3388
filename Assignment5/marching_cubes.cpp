#include "marching_cubes.h"
#include "TriTable.hpp"
#include <glm/glm.hpp>
#include <vector>

using namespace std;
using namespace glm;

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

                const float* edges = vertTable[cubeIndex];  // Fix lookup table usage

                for (int i = 0; edges[i] != -1; i += 3) {
                    for (int j = 0; j < 3; ++j) {
                        int edge = edges[i + j];

                        int edgeIndex1 = edge & 15;
                        int edgeIndex2 = (edge >> 4) & 15;

                        float t = (isovalue - cubeValues[edgeIndex1]) / 
                                  (cubeValues[edgeIndex2] - cubeValues[edgeIndex1]);

                        vec3 p = corners[edgeIndex1] + t * (corners[edgeIndex2] - corners[edgeIndex1]);

                        vertices.push_back(p.x);
                        vertices.push_back(p.y);
                        vertices.push_back(p.z);
                    }
                }
            }
        }
    }

    return vertices;
}
