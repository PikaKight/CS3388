#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

mat4 Camera::getViewMatrix() {
    float x = r * sin(theta) * cos(phi);
    float y = r * sin(phi);
    float z = r * cos(theta) * cos(phi);
    return lookAt(vec3(x, y, z), vec3(0,0,0), vec3(0,1,0));
}

void Camera::processMouse(float dx, float dy) {
    theta += dx * 0.01f;
    phi += dy * 0.01f;
}

void Camera::processScroll(float delta) {
    r = clamp(r - delta * 0.1f, 1.0f, 10.0f);
}

void Camera::processKeyboard(float delta) {
    r = clamp(r - delta * 0.1f, 0.01f, 10.0f);
}
