#ifndef WIRE_FRAME_RENDERER_H
#define WIRE_FRAME_RENDERER_H

#include "Renderer.h"

class WireFrameRenderer : public Renderer
{
public:
    void render(float focalLength, DrawingWindow& window, const Scene& scene) override;
};

#endif