#ifndef PROJECTION_H
#define PROJECTION_H

#include "CanvasPoint.h"
#include "Camera.h"
#include <glm/glm.hpp>

CanvasPoint projectVertexOntoCanvasPoint(float focalLength, const glm::vec3& vertexPosition, const Camera& camera, int height, int width);

#endif