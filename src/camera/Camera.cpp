#include "Camera.h"
#include <glm/glm.hpp>
#include "../other/Constants.h"

Camera::Camera()
    : position(0.0f, 0.0f, 4.0f),
      orientation(glm::mat3(1.0f)),
      focalLength(2.0f),
      orbiting(false) {}

void Camera::reset() {
    position = glm::vec3(0.0f,0.0f,4.0f);
    orientation = glm::mat3(glm::vec3(1.0,0.0,0.0),glm::vec3(0.0,1.0,0.0),glm::vec3(0.0,0.0,1.0));
    orbiting = false;
}

glm::mat3 Camera::rot_y_axis(float r) {
    return glm::mat3(glm::vec3(cos(r),0.0,-sin(r)),glm::vec3(0.0,1.0,0.0),glm::vec3(sin(r),0.0,cos(r)));
}

glm::mat3 Camera::rot_x_axis(float r) {
    return glm::mat3(glm::vec3( 1.0,    0.0,    0.0),glm::vec3( 0.0, cos(r), sin(r)),glm::vec3( 0.0, -sin(r), cos(r)));
}

void Camera::lookAt(glm::vec3 point) {
    glm::vec3 forward = normalize(position - point);
    glm::vec3 right = normalize(cross(glm::vec3(0.0,1.0,0.0), forward));
    glm::vec3 up = cross(forward, right);
    orientation[0] = right;
    orientation[1] = up;
    orientation[2] = forward;
}

void Camera::orbit(bool orbiting) {
    if (orbiting) {
        position = rot_y_axis(-PI/180) * position;
        lookAt(glm::vec3(0,0,0));
    }
}