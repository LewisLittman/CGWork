#include "PointCloudRenderer.h"
#include "Projection.h"

void PointCloudRenderer::render(float focalLength, DrawingWindow& window, const Scene& scene) {
    Colour colour = Colour(255,255,255);
    uint32_t c = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
    for (const auto& triangle : scene.triangles) {
        for (const auto& vertex : triangle.vertices) {
            CanvasPoint point = projectVertexOntoCanvasPoint(focalLength, vertex, scene.camera, window.height, window.width);
            
            if (point.x >= 0 && point.x < window.width &&
                point.y >= 0 && point.y < window.height) {
                window.setPixelColour(point.x, point.y, c);
            }
            
        }
    }
}


