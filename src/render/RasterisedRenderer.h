#ifndef RASTERISED_RENDERER_H
#define RASTERISED_RENDERER_H

#include "Renderer.h"
#include "DepthBuffer.h"

class RasterisedRenderer : public Renderer
{
public:
    void render(float focalLength, DrawingWindow& window, const Scene& scene) override;
    void render(float focalLength, DrawingWindow& window, const Scene& scene, DepthBuffer& depthBuffer);
};

#endif