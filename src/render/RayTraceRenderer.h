#ifndef RAY_TRACE_RENDERER_H
#define RAY_TRACE_RENDERER_H

#include "Renderer.h"
#include "RayTriangleIntersection.h"

class RayTraceRenderer : public Renderer
{
public:
    void render(float focalLength, DrawingWindow& window, const Scene& scene) override;
private:
    RayTriangleIntersection traceRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const Scene& scene, int depth);
    RayTriangleIntersection getClosestIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const Scene& scene);
};


#endif