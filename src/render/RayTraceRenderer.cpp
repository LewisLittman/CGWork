#include "RayTraceRenderer.h"
// #include "ModelTriangle.h"
// #include <glm/glm.hpp>
// #include <glm/ext.hpp>


void RayTraceRenderer::render(float focalLength, DrawingWindow& window, const Scene& scene) {
  for (int x = 0; x < window.width; x++) {
    for (int y = 0; y < window.height; y++) {
    //   float xT = x - window.width / 2;
    //   float yT = window.height / 2 - y;
    // //   if (x % 100 == 0 && y % 100 == 0) std::cout << "x: " << x << ", y: " << y << std::endl;
    //   glm::vec3 transposedPoint = glm::vec3(xT / (window.width / 2) , yT / (window.width / 2), -focalLength);

      float xT = static_cast<float>(x) - static_cast<float>(window.width) / 2.0f;
      float yT = static_cast<float>(window.height) / 2.0f - static_cast<float>(y);
      float scale = static_cast<float>(window.width) / 2.0f;
      glm::vec3 transposedPoint = glm::vec3(xT / scale, yT / scale, -focalLength);


      glm::vec3 rayDirection = normalize(scene.camera.orientation * transposedPoint);
      RayTriangleIntersection closestIntersection = getClosestIntersection(rayDirection, scene);
      uint32_t c = (255 << 24) + (closestIntersection.pointColour.red << 16) + (closestIntersection.pointColour.green << 8) + closestIntersection.pointColour.blue;
      window.setPixelColour(x, y, c);
    }
  }    
}

RayTriangleIntersection RayTraceRenderer::getClosestIntersection(const glm::vec3& rayDirection, const Scene& scene) {
RayTriangleIntersection rayIntersection;
  rayIntersection.hit = false;
  rayIntersection.distanceFromCamera = std::numeric_limits<float>::infinity();
  for (int i = 0; i < scene.triangles.size(); i++) {
    ModelTriangle triangle = scene.triangles[i];
    glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
    glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
    glm::vec3 SPVector = scene.camera.position - triangle.vertices[0];
    glm::mat3 DEMatrix(-rayDirection, e0, e1);
    glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
    float t = possibleSolution.x;
    float u = possibleSolution.y;
    float v = possibleSolution.z;
    if (u >= 0.0 && u <= 1.0 && v >= 0.0 && v <= 1.0 && u + v <= 1.0) {
      if (t < rayIntersection.distanceFromCamera && t > 0)
      {
        rayIntersection.hit = true;
        rayIntersection.distanceFromCamera = t;
        rayIntersection.triangleIndex = i;
        rayIntersection.intersectedTriangle = triangle;
        rayIntersection.intersectionPoint = triangle.vertices[0] + u * e0 + v * e1;
        rayIntersection.u = u;
        rayIntersection.v = v;
        rayIntersection.pointColour = triangle.colour;
      }  
    }
  }
  return rayIntersection;
}