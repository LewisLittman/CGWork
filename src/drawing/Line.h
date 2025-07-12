#ifndef DRAW_H
#define DRAW_H

#include "CanvasPoint.h"
#include "Colour.h"
#include "DrawingWindow.h"
#include "CanvasTriangle.h"

void drawLine(CanvasPoint from, CanvasPoint to, Colour colour, DrawingWindow& window);

CanvasTriangle ensureCorrectOrientation(CanvasTriangle tri);

float edgeFunction(CanvasPoint p0, CanvasPoint p1, CanvasPoint p2);

void baryFillTriangle(CanvasTriangle tri, Colour colour, DrawingWindow &window, std::vector<std::vector<float>> &depthBuffer);

#endif