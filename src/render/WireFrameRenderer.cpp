#include "WireframeRenderer.h"

void WireFrameRenderer::render(float focalLength, DrawingWindow& window, const Scene& scene) {
    window.clear();

    for (const auto& triangle : scene.triangles) {
        for (int i = 0; i < 3; ++i) {
            CanvasPoint p0 = projectVertexOntoCanvasPoint(focalLength, scene.camera, triangle.vertices[i]);
            CanvasPoint p1 = projectVertexOntoCanvasPoint(focalLength, scene.camera, triangle.vertices[(i + 1) % 3]);
            drawLine(p0, p1, triangle.colour, window);
        }
    }
}