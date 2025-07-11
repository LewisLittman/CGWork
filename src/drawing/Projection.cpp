#include "Projection.h"

CanvasPoint projectVertexOntoCanvasPoint(float focalLength, const glm::vec3& vertexPosition, const Camera& camera, int height, int width) {
  glm::vec3 cameraToVertex = camera.position - vertexPosition;
  glm::vec3 adjustedVector =  cameraToVertex * camera.orientation;
  float u = -focalLength * (adjustedVector.x / adjustedVector.z) * width / 2 + width / 2;
  float v = focalLength * (adjustedVector.y / adjustedVector.z) * width / 2 + height / 2;
  CanvasPoint projectedVertex = CanvasPoint(u, v, 1/adjustedVector.z);
  return projectedVertex;
}

