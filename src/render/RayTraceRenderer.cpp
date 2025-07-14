#include "RayTraceRenderer.h"
// #include "ModelTriangle.h"
#include <thread>
// #include <glm/glm.hpp>
// #include <glm/ext.hpp>


void RayTraceRenderer::render(float focalLength, DrawingWindow& window, const Scene& scene) {
  int numThreads = std::thread::hardware_concurrency();
  int workSize = window.width / numThreads;
  std::vector<std::thread> threads;

  auto renderChunk = [&](int startY, int endY) {
    float halfWidth = static_cast<float>(window.width) / 2.0f;
    float halfHeight = static_cast<float>(window.height) / 2.0f;

    for (int x = 0; x < window.width; x++) {
      for (int y = startY; y < endY; y++) {
        float xT = static_cast<float>(x) - halfWidth;
        float yT = halfHeight - static_cast<float>(y);
        glm::vec3 transposedPoint = glm::vec3(xT / halfWidth, yT / halfWidth, -focalLength);
        glm::vec3 rayDirection = glm::normalize(scene.camera.orientation * transposedPoint);
        RayTriangleIntersection closestIntersection = getClosestIntersection(rayDirection, scene);

        if (closestIntersection.hit) {
          uint32_t c = (255 << 24) + (closestIntersection.pointColour.red << 16) + (closestIntersection.pointColour.green << 8) + closestIntersection.pointColour.blue;
          window.setPixelColour(x, y, c);
        } else {
          window.setPixelColour(x, y, 0);
        }
      }
    }
  };

  for (int i = 0; i < numThreads; i++) {
    int startY = i * (window.height / numThreads);
    int endY = (i + 1) * (window.height / numThreads);
    if (i == numThreads - 1) endY = window.height; // Last thread handles any remainder
    threads.emplace_back(renderChunk, startY, endY);
  }

  // Join threads
  for (auto& t : threads) {
    t.join();
  }
}

// RayTriangleIntersection RayTraceRenderer::getClosestIntersection(const glm::vec3& rayDirection, const Scene& scene) {
// RayTriangleIntersection rayIntersection;
//   rayIntersection.hit = false;
//   rayIntersection.distanceFromCamera = std::numeric_limits<float>::infinity();
//   for (int i = 0; i < scene.triangles.size(); i++) {
//     ModelTriangle triangle = scene.triangles[i];
//     glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
//     glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
//     glm::vec3 SPVector = scene.camera.position - triangle.vertices[0];
//     glm::mat3 DEMatrix(-rayDirection, e0, e1);
//     glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
//     float t = possibleSolution.x;
//     float u = possibleSolution.y;
//     float v = possibleSolution.z;
//     if (u >= 0.0 && u <= 1.0 && v >= 0.0 && v <= 1.0 && u + v <= 1.0) {
//       if (t < rayIntersection.distanceFromCamera && t > 0)
//       {
//         rayIntersection.hit = true;
//         rayIntersection.distanceFromCamera = t;
//         rayIntersection.triangleIndex = i;
//         rayIntersection.intersectedTriangle = triangle;
//         rayIntersection.intersectionPoint = triangle.vertices[0] + u * e0 + v * e1;
//         rayIntersection.u = u;
//         rayIntersection.v = v;
//         rayIntersection.pointColour = triangle.colour;
//       }  
//     }
//   }
//   return rayIntersection;
// }

RayTriangleIntersection RayTraceRenderer::getClosestIntersection(const glm::vec3& rayDirection, const Scene& scene) {
    RayTriangleIntersection rayIntersection;
    rayIntersection.hit = false;
    rayIntersection.distanceFromCamera = std::numeric_limits<float>::infinity();

    glm::vec3 rayOrigin = scene.camera.position;

    for (int i = 0; i < scene.triangles.size(); i++) {
        const ModelTriangle& triangle = scene.triangles[i];

        glm::vec3 v0 = triangle.vertices[0];
        glm::vec3 v1 = triangle.vertices[1];
        glm::vec3 v2 = triangle.vertices[2];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;

        glm::vec3 h = glm::cross(rayDirection, edge2);
        float a = glm::dot(edge1, h);
        if (fabs(a) < 1e-6) continue; // Ray is parallel to triangle

        float f = 1.0f / a;
        glm::vec3 s = rayOrigin - v0;
        float u = f * glm::dot(s, h);
        if (u < 0.0f || u > 1.0f) continue;

        glm::vec3 q = glm::cross(s, edge1);
        float v = f * glm::dot(rayDirection, q);
        if (v < 0.0f || u + v > 1.0f) continue;

        float t = f * glm::dot(edge2, q);
        if (t > 0.0f && t < rayIntersection.distanceFromCamera) {
            rayIntersection.hit = true;
            rayIntersection.distanceFromCamera = t;
            rayIntersection.triangleIndex = i;
            rayIntersection.intersectedTriangle = triangle;
            rayIntersection.intersectionPoint = rayOrigin + t * rayDirection;
            rayIntersection.u = u;
            rayIntersection.v = v;
            rayIntersection.pointColour = triangle.colour;
        }
    }

    return rayIntersection;
}