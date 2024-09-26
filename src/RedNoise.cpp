#include <algorithm>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <CanvasPoint.h>

#include "Colour.h"

#define WIDTH 320
#define HEIGHT 240

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) {
	std::vector<float> result;
	float step = (to - from) / (numberOfValues - 1);
	for (int i = 0; i < numberOfValues; i++) {
		float value = from + i * step;
		result.push_back(value);
	}
	return result;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues) {
	std::vector<float> interpolated_x = interpolateSingleFloats(from.x, to.x, numberOfValues);
	std::vector<float> interpolated_y = interpolateSingleFloats(from.y, to.y, numberOfValues);
	std::vector<float> interpolated_z = interpolateSingleFloats(from.z, to.z, numberOfValues);
	std::vector<glm::vec3> result;
	for (int i = 0; i < numberOfValues; i++) {
		float x = interpolated_x[i];
		float y = interpolated_y[i];
		float z = interpolated_z[i];
		result.push_back(glm::vec3(x, y, z));
	}
	return result;
}

void drawLine(CanvasPoint from, CanvasPoint to, Colour colour, DrawingWindow &window) {
	uint32_t c = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
	float xDiff = to.x - from.x;
	float yDiff = to.y - from.y;
	float numberOfSteps = std::max(abs(xDiff), abs(yDiff));
	float xStepSize = xDiff / numberOfSteps;
	float yStepSize = yDiff / numberOfSteps;
	for (float i = 0.0; i <= numberOfSteps; i++) {
		float x = from.x + (i * xStepSize);
		float y = from.y + (i * yStepSize);
		window.setPixelColour(round(x), round(y), c);
	}
}

void draw(DrawingWindow &window) {
	window.clearPixels();
	for (size_t y = 0; y < window.height; y++) {
		for (size_t x = 0; x < window.width; x++) {
			float red = 255;
			float green = 0.0;
			float blue = 255;
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
	drawLine(CanvasPoint(WIDTH / 2, 0), CanvasPoint(WIDTH / 2, HEIGHT - 1), Colour(255,255,255), window);
	drawLine(CanvasPoint(WIDTH / 3, HEIGHT / 2), CanvasPoint(2*WIDTH / 3, HEIGHT / 2), Colour(255,255,255), window);
	drawLine(CanvasPoint(0, 0), CanvasPoint(WIDTH / 2, HEIGHT / 2), Colour(255,255,255), window);
	drawLine(CanvasPoint(WIDTH - 1, 0), CanvasPoint(WIDTH / 2, HEIGHT / 2), Colour(255,255,255), window);
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	// std::vector<glm::vec3> result; //checks 3 element interpolation
	// glm::vec3 from(1.0, 4.0, 9.2);
	// glm::vec3 to(4.0, 1.0, 9.8);
	// result = interpolateThreeElementValues(from, to, 4);
	// for(size_t i=0; i<result.size(); i++) std::cout << to_string(result[i]) << " ";
	// std::cout << std::endl;
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		draw(window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
