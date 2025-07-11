#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera {
public:
    glm::vec3 position;
    glm::mat3 orientation;
    float focalLength;
    bool orbiting;

    Camera();

    void reset();
    void lookAt(glm::vec3 point);
    void orbit(bool enable);
    glm::mat3 rot_y_axis(float r);
    glm::mat3 rot_x_axis(float r);
};

#endif