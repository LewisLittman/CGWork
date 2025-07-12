#include "Fill.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "DepthBuffer.h"

CanvasTriangle ensureCorrectOrientation(CanvasTriangle tri) {
  CanvasPoint p0 = tri.v0();
  CanvasPoint p1 = tri.v1();
  CanvasPoint p2 = tri.v2();

  // Calculate the area (or the z-component of the cross product)
  float area = (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);

  // If the area is negative, reverse the vertex order
  if (area < 0) {
    tri[0] = p2;
    tri[1] = p1;
    tri[2] = p0;
  }
  return CanvasTriangle(tri[0], tri[1], tri[2]);
}

float edgeFunction(CanvasPoint p0, CanvasPoint p1, CanvasPoint p2) {
  return (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);
}

void baryFillTriangle(CanvasTriangle tri, Colour colour, DrawingWindow &window, DepthBuffer& depthBuffer) {
  uint32_t c = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
  CanvasPoint p0 = tri.v0();
  CanvasPoint p1 = tri.v1();
  CanvasPoint p2 = tri.v2();

  float p0p1p2 = edgeFunction(p0, p1, p2);

  if (p0p1p2 >= 0) {
    float minX = floor(glm::min(p0.x, p1.x, p2.x));
    float maxX = ceil(glm::max(p0.x, p1.x, p2.x));
    float minY = floor(glm::min(p0.y, p1.y, p2.y));
    float maxY = ceil(glm::max(p0.y, p1.y, p2.y));

    for (int y = minY; y < maxY; y++) {
       for (int x = minX; x < maxX; x++) {
          CanvasPoint point = CanvasPoint(x,y);
          float p0p1point = edgeFunction(p0, p1, point);
          float p1p2point = edgeFunction(p1, p2, point);
          float p2p0point = edgeFunction(p2, p0, point);

          float w1 = p1p2point / p0p1p2;
          float w2 = p2p0point / p0p1p2;
          float w3 = p0p1point / p0p1p2;

          if (p0p1point >= 0 && p1p2point >= 0 && p2p0point >= 0) {
             float z = p0.depth * w1 + p1.depth * w2 + p2.depth * w3;
             if(x >= 0 && x < window.width && y >= 0 && y < window.height) {
                if (z > depthBuffer.get(x, y)) {
                   depthBuffer.set(x, y, z);
                   window.setPixelColour(x, y, c);
                }
             }
          }
       }
    }
  }
}