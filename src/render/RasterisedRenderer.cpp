#include "RasterisedRenderer.h"
#include "Projection.h"
#include "CanvasPoint.h"
#include "Fill.h"

void RasterisedRenderer::render(float focalLength, DrawingWindow& window, const Scene& scene, DepthBuffer& depthBuffer) {
    for (const auto& triangle : scene.triangles) {
        CanvasPoint p0 = projectVertexOntoCanvasPoint(focalLength, triangle.vertices[0], scene.camera, window.height, window.width);
        CanvasPoint p1 = projectVertexOntoCanvasPoint(focalLength, triangle.vertices[1], scene.camera, window.height, window.width);
        CanvasPoint p2 = projectVertexOntoCanvasPoint(focalLength, triangle.vertices[2], scene.camera, window.height, window.width);
        CanvasTriangle tri = CanvasTriangle(p0, p1, p2);
        baryFillTriangle(ensureCorrectOrientation(tri), triangle.colour, window, depthBuffer);
    }
}

void RasterisedRenderer::render(float, DrawingWindow&, const Scene&) {
    // Stub to satisfy linker; won't be called by your current code.
}