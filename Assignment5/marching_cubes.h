#ifndef MARCHING_CUBES_H
#define MARCHING_CUBES_H

#include <vector>
#include <functional>

std::vector<float> marching_cubes(
    std::function<float(float, float, float)> f,
    float isovalue, float min, float max, float stepsize);

std::vector<float> compute_normals(const std::vector<float>& vertices);

#endif
