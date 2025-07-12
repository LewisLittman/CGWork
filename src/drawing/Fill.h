#ifndef LINE_H
#define LINE_H

#include "CanvasPoint.h"
#include "Colour.h"
#include "DrawingWindow.h"
#include "CanvasTriangle.h"
#include "DepthBuffer.h"

CanvasTriangle ensureCorrectOrientation(CanvasTriangle tri);

float edgeFunction(CanvasPoint p0, CanvasPoint p1, CanvasPoint p2);

void baryFillTriangle(CanvasTriangle tri, Colour colour, DrawingWindow &window, DepthBuffer &depthBuffer);

#endif