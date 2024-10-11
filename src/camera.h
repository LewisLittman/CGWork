//
// Created by Lewis on 10/10/2024.
//

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

using namespace glm;

void reset_camera();
mat3 rot_y_axis(float r);
mat3 rot_x_axis(float r);


#endif //CAMERA_H
