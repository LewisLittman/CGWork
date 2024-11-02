#include "camera.h"
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

extern vec3 cameraPosition;
extern mat3 cameraOrientation;
extern bool orbiting;

const double PI = 3.14159265358979323846;

void reset_camera() {
    cameraPosition = vec3(0.0,0.0,4.0);
    cameraOrientation = mat3(vec3(1.0,0.0,0.0),vec3(0.0,1.0,0.0),vec3(0.0,0.0,1.0));
    orbiting = false;
}

mat3 rot_y_axis(float r) {
    return mat3(vec3(cos(r),0.0,-sin(r)),vec3(0.0,1.0,0.0),vec3(sin(r),0.0,cos(r)));
}

mat3 rot_x_axis(float r) {
    return mat3(vec3( 1.0,    0.0,    0.0),vec3( 0.0, cos(r), sin(r)),vec3( 0.0, -sin(r), cos(r)));
}

void lookAt(vec3 point) {
    vec3 forward = normalize(cameraPosition - point);
    vec3 right = normalize(cross(vec3(0.0,1.0,0.0), forward));
    vec3 up = cross(forward, right);
    cameraOrientation[0] = right;
    cameraOrientation[1] = up;
    cameraOrientation[2] = forward;
}

void orbit(bool orbiting) {
    if (orbiting) {
        cameraPosition = rot_y_axis(-PI/180) * cameraPosition;
        lookAt(vec3(0,0,0));
    }
}