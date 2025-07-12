#include "Line.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

void drawLine(CanvasPoint from, CanvasPoint to, Colour colour, DrawingWindow& window) {
  uint32_t c = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
  float xDiff = to.x - from.x;
  float yDiff = to.y - from.y;
  float numberOfSteps = std::max(abs(xDiff), abs(yDiff));
  float xStepSize = xDiff / numberOfSteps;
  float yStepSize = yDiff / numberOfSteps;
  for (float i = 0.0; i <= numberOfSteps; i++) {
    float x = from.x + (i * xStepSize);
    float y = from.y + (i * yStepSize);
    if (x >= 0 && x < window.width && y >= 0 && y < window.height) {
      window.setPixelColour(round(x), round(y), c);
    }
  }
}
