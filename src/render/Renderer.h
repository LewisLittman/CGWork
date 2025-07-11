#ifndef RENDERER_H
#define RENDERER_H

#include "Scene.h"
#include "DrawingWindow.h"

class Renderer {
public:
    virtual void render(float focalLength, DrawingWindow &window, const Scene &scene) = 0;
    virtual ~Renderer() = default;
};

#endif