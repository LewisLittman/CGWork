//
// Created by Lewis on 10/10/2024.
//

#include "camera.h"
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

extern vec3 cameraPosition;
extern mat3 cameraOrientation;

void reset_camera() {
    cameraPosition = vec3(0.0,0.0,4.0);
    cameraOrientation = mat3(vec3(1.0,0.0,0.0),vec3(0.0,1.0,0.0),vec3(0.0,0.0,1.0));
}

mat3 rot_y_axis(float r) {
     return mat3(vec3(cos(r),0.0,sin(r)),vec3(0.0,1.0,0.0),vec3(-sin(r),0.0,cos(r)));
}

mat3 rot_x_axis(float r) {
    return mat3(vec3( 1.0,    0.0,    0.0),vec3( 0.0, cos(r),-sin(r)),vec3( 0.0, sin(r), cos(r)));
}