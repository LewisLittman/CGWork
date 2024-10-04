#include <algorithm>
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <CanvasPoint.h>
#include <set>
#include <TextureMap.h>
#include <ModelTriangle.h>
#include <unordered_map>

#include "Colour.h"

#define WIDTH 320
#define HEIGHT 240

//parses obj files returning vector<ModelTriangle> with all vertices of each triangle
std::vector<ModelTriangle> parseObj(std::string filename, float scale, std::unordered_map<std::string, Colour> colours) {
	std::ifstream File(filename);
	std::string line;

	std::vector<ModelTriangle> triangles;
	std::vector<glm::vec3> vertices;
	std::string colour;

	while (std::getline(File, line)) {
		if (line == "") continue;
		std::vector<std::string> values = split(line, ' ');
		if (values[0] == "v") {
			glm::vec3 vertex(std::stof(values[1]) * scale, std::stof(values[2]) * scale,
			std::stof(values[3]) * scale);
			vertices.push_back(vertex);
		} else if (values[0] == "f") {
			triangles.push_back(ModelTriangle(vertices[std::stoi(values[1])-1],
			vertices[std::stoi(values[2])-1], vertices[std::stoi(values[3])-1], colours[colour]));
		} else if (values[0] == "usemtl") {
			colour = values[1];
		}
	}
	File.close();
	std::cout << vertices.size() << " vertices" << std::endl;
	std::cout << triangles.size() << " triangles" << std::endl;
	return triangles;
}

//parses mtl files and creates a hash table pairing colour names to colour values
std::unordered_map<std::string, Colour> parseMtl(std::string filename) {
	std::ifstream File(filename);
	std::string line;

	std::unordered_map<std::string, Colour> colours;
	std::string colour;

	while (std::getline(File, line)) {
		if (line == "") continue;

		std::vector<std::string> values = split(line, ' ');
		if (values[0] == "newmtl") {
			colour = values[1];
		} else if (values[0] == "Kd") {
			colours.insert({colour, Colour(int(stof(values[1]) * 255),
			int(stof(values[2]) * 255), int(stof(values[3]) * 255))});
		}
	}
	File.close();
	return colours;
}

CanvasPoint projectVertexOntoCanvasPoint(glm::vec3 cameraPosition, float focalLength, glm::vec3 vertexPosition) {
	float u = -focalLength * ((vertexPosition.x - cameraPosition.x) / (vertexPosition.z - cameraPosition.z)) * 160 + WIDTH / 2;
	float v = focalLength * ((vertexPosition.y - cameraPosition.y) / (vertexPosition.z - cameraPosition.z)) * 160 + HEIGHT / 2;
	CanvasPoint projectedVertex = CanvasPoint(round(u), round(v));
	return projectedVertex;
}

void pointCloud(glm::vec3 cameraPosition, float focalLength, DrawingWindow &window, std::vector<ModelTriangle> modelTriangles) {
	Colour colour = Colour(255,255,255);
	uint32_t c = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
	for (int i = 0; i < modelTriangles.size(); i++) {
		ModelTriangle triangle = modelTriangles[i];
		for (int j = 0; j < triangle.vertices.size() ; j++) {
			CanvasPoint point = projectVertexOntoCanvasPoint(cameraPosition, focalLength, triangle.vertices[j]);
			window.setPixelColour(point.x, point.y, c);
		}
	}
}

void wireFrameRender() {

}


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

std::vector<CanvasPoint> pixelsOnLine(CanvasPoint from, CanvasPoint to) {
	std::vector<CanvasPoint> pixels;
	float xDiff = to.x - from.x;
	float yDiff = to.y - from.y;
	float numberOfSteps = std::max(abs(xDiff), abs(yDiff));
	float xStepSize = xDiff / numberOfSteps;
	float yStepSize = yDiff / numberOfSteps;
	for (float i = 0.0; i <= numberOfSteps; i++) {
		float x = from.x + (i * xStepSize);
		float y = from.y + (i * yStepSize);
		CanvasPoint pixel = CanvasPoint(round(x), round(y));
		pixels.push_back(pixel);
	}
	return pixels;
}

std::vector<TexturePoint> pixelsOnTextureLine(TexturePoint from, TexturePoint to) {
	std::vector<TexturePoint> pixels;
	float xDiff = to.x - from.x;
	float yDiff = to.y - from.y;
	float numberOfSteps = std::max(abs(xDiff), abs(yDiff));
	float xStepSize = xDiff / numberOfSteps;
	float yStepSize = yDiff / numberOfSteps;
	for (float i = 0.0; i <= numberOfSteps; i++) {
		float x = from.x + (i * xStepSize);
		float y = from.y + (i * yStepSize);
		TexturePoint pixel = TexturePoint(round(x), round(y));
		pixels.push_back(pixel);
	}
	return pixels;
}

void drawTriangle(CanvasTriangle triangle, Colour colour, DrawingWindow &window) {
	drawLine(triangle.v0(), triangle.v1(), colour, window);
	drawLine(triangle.v1(), triangle.v2(), colour, window);
	drawLine(triangle.v2(), triangle.v0(), colour, window);
}

void randomTriangle(DrawingWindow &window) {
	CanvasPoint p0 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasPoint p1 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasPoint p2 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	drawTriangle(CanvasTriangle(p0,p1,p2), Colour(255,255,255), window);
}

void fillHalfTriangle(CanvasTriangle triangle, Colour colour, DrawingWindow &window) {
	std::vector<CanvasPoint> line1 = pixelsOnLine(triangle.v0(), triangle.v1());
	std::vector<CanvasPoint> line2 = pixelsOnLine(triangle.v0(), triangle.v2());
	for (int i = 0; i < line1.size(); i++) {
		for (int j = 0; j < line2.size(); j++) {
			if (line1[i].y == line2[j].y) {
				drawLine(line1[i], line2[j], colour, window);
			}
		}
	}
}

void fillTriangle(CanvasTriangle triangle, Colour colour, DrawingWindow &window) {
	//initialise vertices
	CanvasPoint top = triangle.v0();
	CanvasPoint mid = triangle.v1();
	CanvasPoint bot = triangle.v2();

	//bubble sort by y pos
	if (bot.y < mid.y) {
		std::swap(bot, mid);
	}
	if (mid.y < top.y) {
		std::swap(mid, top);
	}
	if (bot.y < mid.y) {
		std::swap(bot, mid);
	}

	//find opposite x by using the ratio from top to mid y and multiplying it by the distance between
	//bot and top x and adding to the top x
	float opx = top.x + (mid.y - top.y) / (bot.y - top.y) * (bot.x - top.x);
	CanvasPoint opMid = CanvasPoint(opx, mid.y);
	CanvasTriangle topTriangle = CanvasTriangle(top, mid, opMid);
	CanvasTriangle botTriangle = CanvasTriangle(bot, mid, opMid);

	fillHalfTriangle(topTriangle, colour, window);
	fillHalfTriangle(botTriangle, colour, window);
	drawTriangle(triangle, Colour(255,255,255), window);
}

void drawFillTriangle(DrawingWindow &window) {
	CanvasPoint p0 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasPoint p1 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasPoint p2 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasTriangle triangle = CanvasTriangle(p0, p1, p2);
	fillTriangle(triangle, Colour(rand() % 256,rand() % 256,rand() % 256), window);
}

void textureLine(CanvasPoint from, CanvasPoint to, TextureMap texture, DrawingWindow &window) {
	std::vector<TexturePoint> pixelsOnTexture = pixelsOnTextureLine(from.texturePoint, to.texturePoint);
	int sizeOfLine = to.x - from.x;
	for (float i = 0; i < sizeOfLine; i++) {
		int texturePixel = (i / sizeOfLine) * pixelsOnTexture.size();
		uint32_t colour = texture.pixels[round(pixelsOnTexture[texturePixel].x) + round(pixelsOnTexture[texturePixel].y  * texture.width)];
		window.setPixelColour(from.x + i, from.y, colour);
	}
}

void textureHalfTriangle(CanvasTriangle triangle, TextureMap texture, DrawingWindow &window) {
	std::vector<CanvasPoint> line1 = pixelsOnLine(triangle.v0(), triangle.v1());
	std::vector<CanvasPoint> line2 = pixelsOnLine(triangle.v0(), triangle.v2());
	for (int i = 0; i < line1.size(); i++) {
		for (int j = 0; j < line2.size(); j++) {
			if (line1[i].y == line2[j].y) {
				float line1ratio = float(i) / line1.size();
				float line2ratio = float(j) / line2.size();
				float texturepoint1x = triangle.v0().texturePoint.x + (triangle.v1().texturePoint.x - triangle.v0().texturePoint.x) * line1ratio;
				float texturepoint1y = triangle.v0().texturePoint.y + (triangle.v1().texturePoint.y - triangle.v0().texturePoint.y) * line1ratio;
				float texturepoint2x = triangle.v0().texturePoint.x + (triangle.v2().texturePoint.x - triangle.v0().texturePoint.x) * line2ratio;
				float texturepoint2y = triangle.v0().texturePoint.y + (triangle.v2().texturePoint.y - triangle.v0().texturePoint.y) * line2ratio;
				line1[i].texturePoint = TexturePoint(texturepoint1x, texturepoint1y);
				line2[j].texturePoint = TexturePoint(texturepoint2x, texturepoint2y);
				textureLine(line1[i], line2[j], texture, window);
			}
		}
	}
}

void textureTriangle(TextureMap texture, CanvasTriangle triangle, DrawingWindow &window) {
	//initialise vertices
	CanvasPoint top = triangle.v0();
	CanvasPoint mid = triangle.v1();
	CanvasPoint bot = triangle.v2();

	//bubble sort by y pos
	if (bot.y < mid.y) {
		std::swap(bot, mid);
	}
	if (mid.y < top.y) {
		std::swap(mid, top);
	}
	if (bot.y < mid.y) {
		std::swap(bot, mid);
	}

	//find opposite x by using the ratio from top to mid y and multiplying it by the distance between
	//bot and top x and adding to the top x
	float opx = top.x + (mid.y - top.y) / (bot.y - top.y) * (bot.x - top.x);
	CanvasPoint opMid = CanvasPoint(opx, mid.y);

	//find corresponding texturepoint for the new midpoint
	float ratio = (opMid.y - top.y) / (bot.y - top.y);
	opMid.texturePoint.x = top.texturePoint.x + ratio * (bot.texturePoint.x - top.texturePoint.x);
	opMid.texturePoint.y = top.texturePoint.y + ratio * (bot.texturePoint.y - top.texturePoint.y);

	CanvasTriangle topTriangle = CanvasTriangle(top, mid, opMid);
	CanvasTriangle botTriangle = CanvasTriangle(bot, mid, opMid);

	textureHalfTriangle(topTriangle, texture, window);
	textureHalfTriangle(botTriangle, texture, window);
	drawTriangle(triangle, Colour(255,255,255), window);
}

void draw(DrawingWindow &window) {
	window.clearPixels();
	for (size_t y = 0; y < window.height; y++) {
		for (size_t x = 0; x < window.width; x++) {
			float red = 250;
			float green = 0.0;
			float blue = 255;
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == SDLK_u) randomTriangle(window);
		else if (event.key.keysym.sym == SDLK_f) drawFillTriangle(window);
		else if (event.key.keysym.sym == SDLK_t) {
			TextureMap texture = TextureMap("../assets/texture.ppm");
			CanvasPoint p0 = CanvasPoint(160, 10);
			CanvasPoint p1 = CanvasPoint(300, 230);
			CanvasPoint p2 = CanvasPoint(10, 150);
			p0.texturePoint = TexturePoint(195,5);
			p1.texturePoint = TexturePoint(395,380);
			p2.texturePoint = TexturePoint(65,330);

			CanvasTriangle triangle = CanvasTriangle(p0, p1, p2);
			textureTriangle(texture, triangle, window);
		}
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	glm::vec3 cameraPosition = glm::vec3(0, 0, 4.0);
	std::vector<ModelTriangle> modelTriangles = parseObj("../models/cornell-box.obj", 0.35, parseMtl("../models/cornell-box.mtl"));

	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	//draw(window);
	pointCloud(cameraPosition, 2.0, window, modelTriangles);
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
