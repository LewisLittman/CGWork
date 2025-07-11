#ifndef POINTCLOUDRENDERER_H
#define POINTCLOUDRENDERER_H

#include "Renderer.h"
#include "Scene.h"
#include "DrawingWindow.h"

class PointCloudRenderer : public Renderer {
public:
    void render(float focalLength, DrawingWindow& window, const Scene& scene) override;
};

#endif