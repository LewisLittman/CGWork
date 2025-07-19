#include "RayTraceRenderer.h"
// #include "ModelTriangle.h"
#include <thread>
#include <glm/glm.hpp>
#include <glm/ext.hpp>


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
        RayTriangleIntersection rayHit = traceRay(scene.camera.position, rayDirection, scene, 3);

        if (rayHit.hit) {
          float shadowIntensity = getShadowIntensity(rayHit, scene);
          uint32_t c = (255 << 24) + (int(rayHit.pointColour.red * shadowIntensity) << 16) + (int(rayHit.pointColour.green * shadowIntensity) << 8) + int(rayHit.pointColour.blue * shadowIntensity);
          window.setPixelColour(x, y, c);
        } else {
          uint32_t envMapColour = getEnvMapColour(rayDirection, scene);
          window.setPixelColour(x, y, envMapColour);
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

RayTriangleIntersection RayTraceRenderer::traceRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const Scene& scene, int depth) {
    if (depth <= 0) {
        return {}; 
    }

    RayTriangleIntersection closestIntersection = getClosestIntersection(rayOrigin, rayDirection, scene);
    if (!closestIntersection.hit) {
        return closestIntersection;
    }

    if (closestIntersection.intersectedTriangle.mirror) {
        glm::vec3 surfaceNormal = closestIntersection.intersectedTriangle.normal;
        glm::vec3 reflectionRay = rayDirection - 2 * surfaceNormal * dot(rayDirection, surfaceNormal);

        return traceRay(closestIntersection.intersectionPoint, reflectionRay, scene, depth - 1);
    } 

    if (!closestIntersection.intersectedTriangle.mirror) {

    }

    return closestIntersection;
}

RayTriangleIntersection RayTraceRenderer::getClosestIntersection(const glm::vec3& rayOrigin,const glm::vec3& rayDirection, const Scene& scene) {
    RayTriangleIntersection rayIntersection;
    rayIntersection.hit = false;
    rayIntersection.distanceFromCamera = std::numeric_limits<float>::infinity();

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
        if (t > 0.0001f && t < rayIntersection.distanceFromCamera) {
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

float RayTraceRenderer::getShadowIntensity(RayTriangleIntersection intersection, const Scene& scene) {
    float shadowIntensity = 1.0f;

    for (const auto& light : scene.lights) {
        glm::vec3 pointToLight = light - intersection.intersectionPoint;
        glm::vec3 offsetOrigin = intersection.intersectionPoint + 0.001f * intersection.intersectedTriangle.normal;
        RayTriangleIntersection shadowRayHit = traceRay(offsetOrigin, glm::normalize(pointToLight), scene, 1);
        if (shadowRayHit.hit && glm::length(shadowRayHit.intersectionPoint - intersection.intersectionPoint) < glm::length(pointToLight)) {
            shadowIntensity *= 0.5f; 
        }
    }
    return shadowIntensity;
}

uint32_t RayTraceRenderer::getEnvMapColour(const glm::vec3& rayDirection, const Scene& scene) {
    float absX = fabs(rayDirection.x);
    float absY = fabs(rayDirection.y);
    float absZ = fabs(rayDirection.z);

    float u, v;
    std::string face;
    if (absX >= absY && absX >= absZ) {
      if (rayDirection.x > 0) {
        u = rayDirection.z / absX;
        v = -rayDirection.y / absX;
        face = "px";
      } else {
        u = -rayDirection.z / absX;
        v = -rayDirection.y / absX;
        face = "nx";
      }
    } else if (absY >= absX && absY >= absZ) {
      if (rayDirection.y > 0) {
        u = rayDirection.x / absY;
        v = -rayDirection.z / absY;
        face = "py";
      } else {
        u = rayDirection.x / absY;
        v = rayDirection.z / absY;
        face = "ny";
      }
    } else {
      if (rayDirection.z > 0) {
        u = -rayDirection.x / absZ;
        v = -rayDirection.y / absZ;
        face = "nz";
      } else {
        u = rayDirection.x / absZ;
        v = -rayDirection.y / absZ;
        face = "pz";
      }
    }
    
    float xRatio = 0.5f * (u + 1.0f);
    float yRatio = 0.5f * (v + 1.0f);

    const TextureMap& envmap = scene.textures.at(face);

    float x = round(xRatio * (envmap.width - 1));
    float y = round(yRatio * (envmap.height - 1));

    float texturePixel = x + y * envmap.width;
    return envmap.pixels[texturePixel];
}
