#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

using namespace glm;

class Camera {
public:
    float r = 5.0f, theta = 0.0f, phi = 0.0f;
    mat4 getViewMatrix();
    void processMouse(float dx, float dy);
    void processScroll(float delta);
    void processKeyboard(float delta);
};

#endif
