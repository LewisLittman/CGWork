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
#include "camera.h"
#include "RayTriangleIntersection.h"


#define WIDTH 600
#define HEIGHT 600


using namespace glm;
using namespace std;


vec3 cameraPosition;
mat3 cameraOrientation;
const double PI = 3.14159265358979323846;
bool orbiting;
float focalLength;
int renderMode;
bool textureToggle;

enum ShadingMode {
  NONE = 0,
  GOURAD = 1,
  PHONG = 2
};

uint32_t pack_colour(Colour colour, float intensity) {
  return (255 << 24) + (int(colour.red * intensity) << 16) + (int(colour.green * intensity) << 8) + int(colour.blue * intensity);
}

uint32_t pack_colour(uint32_t colour, float intensity) {
  float r = (colour >> 16) & 0xff;
  float g = (colour >> 8) & 0xff;
  float b = colour & 0xff;
  return (255 << 24) + (int(r * intensity) << 16) + (int(g * intensity) << 8) + int(b * intensity);
}

Colour convert_colour_type(uint32_t colour) {
  int r = (colour >> 16) & 0xff;
  int g = (colour >> 8) & 0xff;
  int b = colour & 0xff;
  return Colour(r, g, b);
}


vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) {
  vector<float> result;
  float step = (to - from) / (numberOfValues - 1);
  for (int i = 0; i < numberOfValues; i++) {
    float value = from + i * step;
    result.push_back(value);
  }
  return result;
}


vector<vec3> interpolateThreeElementValues(vec3 from, vec3 to, int numberOfValues) {
  vector<float> interpolated_x = interpolateSingleFloats(from.x, to.x, numberOfValues);
  vector<float> interpolated_y = interpolateSingleFloats(from.y, to.y, numberOfValues);
  vector<float> interpolated_z = interpolateSingleFloats(from.z, to.z, numberOfValues);
  vector<vec3> result;
  for (int i = 0; i < numberOfValues; i++) {
    float x = interpolated_x[i];
    float y = interpolated_y[i];
    float z = interpolated_z[i];
    result.push_back(vec3(x, y, z));
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
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
      window.setPixelColour(round(x), round(y), c);
    }
  }
}


void drawDepthLine(CanvasPoint from, CanvasPoint to, Colour colour, DrawingWindow &window, std::vector<std::vector<float>> &depthBuffer) {
  uint32_t c = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
  float xDiff = to.x - from.x;
  float yDiff = to.y - from.y;
  float dDiff = to.depth - from.depth;
  float numberOfSteps = std::max(abs(xDiff), abs(yDiff));
  float xStepSize = xDiff / numberOfSteps;
  float yStepSize = yDiff / numberOfSteps;
  float dStepSize = dDiff / numberOfSteps;
  for (float i = 0.0; i <= numberOfSteps; i++) {
    float x = from.x + (i * xStepSize);
    float y = from.y + (i * yStepSize);
    float d = from.depth + (i * dStepSize);
    if(x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
       if (d > depthBuffer[x][y]) {
          depthBuffer[x][y] = d;
          window.setPixelColour(x, y, c);
       }
    }
  }
}


std::vector<CanvasPoint> pixelsOnLine(CanvasPoint from, CanvasPoint to) {
  std::vector<CanvasPoint> pixels;
  float xDiff = to.x - from.x;
  float yDiff = to.y - from.y;
  float dDiff = to.depth - from.depth;
  float numberOfSteps = std::max(abs(xDiff), abs(yDiff));
  float xStepSize = xDiff / numberOfSteps;
  float yStepSize = yDiff / numberOfSteps;
  float dStepSize = dDiff / numberOfSteps;
  for (float i = 0.0; i <= numberOfSteps; i++) {
    float x = from.x + (i * xStepSize);
    float y = from.y + (i * yStepSize);
    float d = from.depth + (i * dStepSize);
    CanvasPoint pixel = CanvasPoint(x, y, d); //removed round statements around x and y
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


void drawTriangle(CanvasTriangle triangle, Colour colour, DrawingWindow &window, std::vector<std::vector<float>> &depthBuffer) {
  drawDepthLine(triangle.v0(), triangle.v1(), colour, window, depthBuffer);
  drawDepthLine(triangle.v1(), triangle.v2(), colour, window, depthBuffer);
  drawDepthLine(triangle.v2(), triangle.v0(), colour, window, depthBuffer);
}


void fillHalfTriangle(CanvasTriangle triangle, Colour colour, DrawingWindow &window, std::vector<std::vector<float>> &depthBuffer) {
  std::vector<CanvasPoint> line1 = pixelsOnLine(triangle.v0(), triangle.v1());
  std::vector<CanvasPoint> line2 = pixelsOnLine(triangle.v0(), triangle.v2());
  for (size_t i = 0; i < line1.size(); i++) {
    for (size_t j = 0; j < line2.size(); j++) {
       if (line1[i].y == line2[j].y) {
          drawDepthLine(line1[i], line2[j], colour, window, depthBuffer);
       }
    }
  }
}


void fillTriangle(CanvasTriangle triangle, Colour colour, DrawingWindow &window, std::vector<std::vector<float>> &depthBuffer) {
  //initialise vertices
  CanvasPoint top = triangle.v0();
  CanvasPoint mid = triangle.v1();
  CanvasPoint bot = triangle.v2();

  //bubble sort by y pos
  if (bot.y < mid.y) { swap(bot, mid); }
  if (mid.y < top.y) { swap(mid, top); }
  if (bot.y < mid.y) { swap(bot, mid); }

  //find opposite x by using the ratio from top to mid y and multiplying it by the distance between
  //bot and top x and adding to the top x
  float opx = top.x + (mid.y - top.y) / (bot.y - top.y) * (bot.x - top.x);
  CanvasPoint opMid = CanvasPoint(opx, mid.y);
  //find depth value of opposite mid point
  opMid.depth = top.depth + (mid.y - top.y) / (bot.y - top.y) * (bot.depth - top.depth);
  CanvasTriangle topTriangle = CanvasTriangle(top, mid, opMid);
  CanvasTriangle botTriangle = CanvasTriangle(bot, mid, opMid);

  fillHalfTriangle(topTriangle, colour, window, depthBuffer);
  fillHalfTriangle(botTriangle, colour, window, depthBuffer);
}

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

void baryFillTriangle(CanvasTriangle tri, Colour colour, DrawingWindow &window, vector<vector<float>> &depthBuffer) {
  uint32_t c = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
  CanvasPoint p0 = tri.v0();
  CanvasPoint p1 = tri.v1();
  CanvasPoint p2 = tri.v2();

  float p0p1p2 = edgeFunction(p0, p1, p2);

  if (p0p1p2 >= 0) {
    float minX = floor(min(p0.x, p1.x, p2.x));
    float maxX = ceil(max(p0.x, p1.x, p2.x));
    float minY = floor(min(p0.y, p1.y, p2.y));
    float maxY = ceil(max(p0.y, p1.y, p2.y));

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
             if(x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                if (z > depthBuffer[x][y]) {
                   depthBuffer[x][y] = z;
                   window.setPixelColour(x, y, c);
                }
             }
          }
       }
    }
  }
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
  for (size_t i = 0; i < line1.size(); i++) {
    for (size_t j = 0; j < line2.size(); j++) {
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

void textureTriangle(TextureMap texture, CanvasTriangle triangle, DrawingWindow &window, std::vector<std::vector<float>> &depthBuffer) {
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
  drawTriangle(triangle, Colour(255,255,255), window, depthBuffer);
}

//parses obj files returning vector<ModelTriangle> with all vertices of each triangle
std::vector<ModelTriangle> parseObj(std::string filename, float scale, std::unordered_map<std::string, Colour> colours, vec3 offset, bool shadows, int shadingMode) {
  std::ifstream File(filename);
  std::string line;
  std::vector<ModelTriangle> triangles;
  std::vector<vec3> vertices;
  std::vector<vec3> vertexNormals; // vector to store normals for each vertex
  vector<TexturePoint> texture_points; // vector to store texture points for each triangle
  std::string colour;

  while (std::getline(File, line)) {
      if (line.empty()) continue;
      std::vector<std::string> values = split(line, ' ');
      if (values[0] == "v") {
          vec3 vertex(stof(values[1]) * scale, stof(values[2]) * scale, stof(values[3]) * scale);
          vertex += offset;
          vertices.push_back(vertex);
          vertexNormals.emplace_back(0.0f, 0.0f, 0.0f); // Initialize normals to zero
      } else if (values[0] == "vt") {
          texture_points.push_back(TexturePoint(stof(values[1]), stof(values[2])));
      } else if (values[0] == "f") {
          int i1 = std::stoi(values[1]) - 1;
          int i2 = std::stoi(values[2]) - 1;
          int i3 = std::stoi(values[3]) - 1;
          vector<string> v1 = split(values[1], '/');
          vector<string> v2 = split(values[2], '/');
          vector<string> v3 = split(values[3], '/');
          ModelTriangle triangle(vertices[stoi(v1[0]) - 1], vertices[stoi(v2[0]) - 1], vertices[stoi(v3[0]) - 1], colours[colour]);
          triangle.shadows = shadows;
          triangle.shadingMode = shadingMode;
          if (colour == "Mirror") {
            triangle.mirror = true;
          }
          if (colour == "NormalMap") {
            triangle.normalMap = true;
            triangle.texture = true;
          }
          if (colour == "Texture") {
            triangle.texture = true;
          }
          if (v1.size() > 1 && !v1[1].empty() && v2.size() > 1 && !v2[1].empty() && v3.size() > 1 && !v3[1].empty()) {
            triangle.texturePoints[0] = texture_points[stoi(v1[1]) - 1];
            triangle.texturePoints[1] = texture_points[stoi(v2[1]) - 1];
            triangle.texturePoints[2] = texture_points[stoi(v3[1]) - 1];
          }
          triangles.push_back(triangle);
          // Calculate face normal and add it to vertex normals
          vec3 faceNormal = normalize(cross(vertices[i1] - vertices[i3], vertices[i2] - vertices[i3]));
          triangles.back().normal = faceNormal;
          vertexNormals[i1] += faceNormal;
          vertexNormals[i2] += faceNormal;
          vertexNormals[i3] += faceNormal;
      } else if (values[0] == "usemtl") {
          colour = values[1];
      }
  }
  File.close();

  // Normalize all vertex normals
  for (auto& normal : vertexNormals) {
      normal = normalize(normal);
  }
  // Assign calculated normals to each triangle’s vertices
  for (auto& triangle : triangles) {
      for (int i = 0; i < 3; ++i) {
          for (int j = 0; j < vertices.size(); ++j) {
              if (triangle.vertices[i] == vertices[j]) {
                  triangle.vertexNormals[i] = vertexNormals[j];
                  break;
              }
          }
      }
  }
  return triangles;
}

//parses mtl files and creates a hash table pairing colour names to colour values
std::unordered_map<std::string, Colour> parseMtl(std::string filename) {
  std::ifstream File(filename);
  std::string line;
  std::unordered_map<std::string, Colour> colours;
  std::string colourName;

  while (std::getline(File, line)) {
    if (line == "") continue;
    std::vector<std::string> values = split(line, ' ');
    if (values[0] == "newmtl") {
       colourName = values[1];
    } else if (values[0] == "Kd") {
       colours.insert({colourName, Colour(int(stof(values[1]) * 255),
       int(stof(values[2]) * 255), int(stof(values[3]) * 255))});
    } else if (values[0] == "map_Kd") {
      Colour colour = colours[colourName];
      colour.name = values[1];
      colours[colourName] = colour;
    }
  }
  File.close();
  return colours;
}

CanvasPoint projectVertexOntoCanvasPoint(float focalLength, vec3 vertexPosition) {
  vec3 cameraToVertex = cameraPosition - vertexPosition;
  vec3 adjustedVector =  cameraToVertex * cameraOrientation;
  float u = -focalLength * (adjustedVector.x / adjustedVector.z) * WIDTH / 2 + WIDTH / 2;
  float v = focalLength * (adjustedVector.y / adjustedVector.z) * HEIGHT / 2 + HEIGHT / 2;
  CanvasPoint projectedVertex = CanvasPoint(u, v, 1/adjustedVector.z);
  return projectedVertex;
}

void pointCloud(float focalLength, DrawingWindow &window, std::vector<ModelTriangle> modelTriangles) {
  Colour colour = Colour(255,255,255);
  uint32_t c = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
  for (size_t i = 0; i < modelTriangles.size(); i++) {
    ModelTriangle triangle = modelTriangles[i];
    for (size_t j = 0; j < triangle.vertices.size() ; j++) {
       CanvasPoint point = projectVertexOntoCanvasPoint(focalLength, triangle.vertices[j]);
       window.setPixelColour(point.x, point.y, c);
    }
  }
}

void wireFrameRender(float focalLength, DrawingWindow &window, std::vector<ModelTriangle> modelTriangles) {
  Colour colour = Colour(255,255,255);
  std::vector<std::vector<float>> depthBuffer(WIDTH, std::vector<float>(HEIGHT, 0.0));
  for (size_t i = 0; i < modelTriangles.size(); i++) {
    ModelTriangle triangle = modelTriangles[i];
    CanvasPoint p0 = projectVertexOntoCanvasPoint(focalLength, triangle.vertices[0]);
    CanvasPoint p1 = projectVertexOntoCanvasPoint(focalLength, triangle.vertices[1]);
    CanvasPoint p2 = projectVertexOntoCanvasPoint(focalLength, triangle.vertices[2]);
    // drawDepthLine(p0, p1, colour, window, depthBuffer);
    // drawDepthLine(p1, p2, colour, window, depthBuffer);
    // drawDepthLine(p2, p0, colour, window, depthBuffer);

    drawLine(p0, p1, colour, window);
    drawLine(p1, p2, colour, window);
    drawLine(p2, p0, colour, window);
  }
}

void rasterisedRender(float focalLength, DrawingWindow &window, std::vector<ModelTriangle> modelTriangles) {
  std::vector<std::vector<float>> depthBuffer(WIDTH, std::vector<float>(HEIGHT, 0.0));
  for (int i = 0; i < modelTriangles.size(); i++) {
    ModelTriangle triangle = modelTriangles[i];
    CanvasPoint p0 = projectVertexOntoCanvasPoint(focalLength, triangle.vertices[0]);
    CanvasPoint p1 = projectVertexOntoCanvasPoint(focalLength, triangle.vertices[1]);
    CanvasPoint p2 = projectVertexOntoCanvasPoint(focalLength, triangle.vertices[2]);
    CanvasTriangle tri = CanvasTriangle(p0, p1, p2);
    // fillTriangle(tri, triangle.colour, window, depthBuffer);
    baryFillTriangle(ensureCorrectOrientation(tri), triangle.colour, window, depthBuffer);
  }
}


float proximityLighting(RayTriangleIntersection point, vec3 light, float lightStrength) {
  float distance = length(light - point.intersectionPoint);
  float intensity = lightStrength / (4 * PI * distance * distance);
  if (intensity < 0) return 0;
  return intensity;
}

float AoILighting(RayTriangleIntersection point, vec3 light) {
  vec3 lightRay = normalize(vec3(light - point.intersectionPoint));
  float intensity = dot(lightRay, point.intersectedTriangle.normal);
  if (intensity < 0) return 0;
  return intensity;
}

float specularLighting(RayTriangleIntersection point, vec3 light) {
  vec3 lightRay = normalize(vec3(point.intersectionPoint - light));
  vec3 reflectionRay = normalize(lightRay - 2 * point.intersectedTriangle.normal * dot(lightRay, point.intersectedTriangle.normal));
  vec3 viewRay = normalize(cameraPosition - point.intersectionPoint);
  float intensity = pow(dot(viewRay, reflectionRay), 256);
  if (intensity < 0) return 0;
  return intensity;
}

float combinedLighting(RayTriangleIntersection point, vec3 light) {
  float proximityLightingIntensity = proximityLighting(point, light, 20);
  float AoILightIntensity = AoILighting(point, light);
  float specularLightingIntensity = specularLighting(point, light);
  float combinedIntensity = 0.4 * proximityLightingIntensity + 0.7 * AoILightIntensity + 0.5 * specularLightingIntensity;
  if (combinedIntensity < 0.2) return 0.2;
  if (combinedIntensity > 1) return 1;
  return combinedIntensity;
}

float gourad(RayTriangleIntersection point, vec3 light) {
  vector<float> brightness;
  for (int i = 0; i < point.intersectedTriangle.vertices.size(); i++) {
    //proximity lighting for each vertex
    float distance = length(light - point.intersectedTriangle.vertices[i]);
    float proxIntensity = 20 / (4 * PI * distance * distance); //20 = light strength
    if (proxIntensity > 1) proxIntensity = 1;
    if (proxIntensity < 0) proxIntensity = 0;

    //AoI lighting for each vertex
    vec3 AoIlightRay = normalize(vec3(light - point.intersectedTriangle.vertices[i]));
    float AoIintensity = dot(AoIlightRay, point.intersectedTriangle.vertexNormals[i]);
    if(AoIintensity > 1) AoIintensity = 1;
    if (AoIintensity < 0) AoIintensity = 0;

    //Specular lighting for each vertex
    vec3 specLightRay = normalize(vec3(point.intersectedTriangle.vertices[i] - light));
    vec3 reflectionRay = normalize(specLightRay - 2 * point.intersectedTriangle.vertexNormals[i] * dot(specLightRay, point.intersectedTriangle.vertexNormals[i]));
    vec3 viewRay = normalize(cameraPosition - point.intersectedTriangle.vertices[i]);
    float specIntensity = pow(dot(viewRay, reflectionRay), 256);
    if (specIntensity > 1) specIntensity = 1;
    if (specIntensity < 0) specIntensity = 0;

    float combinedIntensity = 0.6 * proxIntensity + 0.7 * AoIintensity + 0.4 * specIntensity;
    if (combinedIntensity < 0.2) combinedIntensity = 0.2;
    if (combinedIntensity > 1) combinedIntensity = 1;
    brightness.push_back(combinedIntensity);
  }
  float interpolatedCombinedBrightness = (1 - point.u - point.v) * brightness[0] + point.u * brightness[1] + point.v * brightness[2];
  return interpolatedCombinedBrightness;
}

float phong(RayTriangleIntersection point, vec3 light) {
  vec3 pointNormal = (1 - point.u - point.v) * point.intersectedTriangle.vertexNormals[0] + point.u * point.intersectedTriangle.vertexNormals[1] + point.v * point.intersectedTriangle.vertexNormals[2];
  // pointNormal = normalize(pointNormal);
  //proximity lighting for the point
  float distance = length(light - point.intersectionPoint);
  float proxIntensity = 20 / (4 * PI * distance * distance); //20 = light strength
  if (proxIntensity > 1) proxIntensity = 1;
  if (proxIntensity < 0) proxIntensity = 0;
  //AoI lighting for the point
  vec3 AoIlightRay = normalize(vec3(light - point.intersectionPoint));
  float AoIintensity = dot(AoIlightRay, pointNormal);
  if(AoIintensity > 1) AoIintensity = 1;
  if (AoIintensity < 0) AoIintensity = 0;
  //Specular lighting for each vertex
  vec3 specLightRay = normalize(vec3(point.intersectionPoint- light));
  vec3 reflectionRay = normalize(specLightRay - 2 * pointNormal * dot(specLightRay, pointNormal));
  vec3 viewRay = normalize(cameraPosition - point.intersectionPoint);
  float specIntensity = pow(dot(viewRay, reflectionRay), 256);
  if (specIntensity > 1) specIntensity = 1;
  if (specIntensity < 0) specIntensity = 0;

  float combinedIntensity = 0.7 * proxIntensity + 0.5 * AoIintensity + 0.3 * specIntensity;
  if (combinedIntensity < 0.2) combinedIntensity = 0.2;
  if (combinedIntensity > 1) combinedIntensity = 1;
  return combinedIntensity;
}

uint32_t texturePixel(RayTriangleIntersection point, const TextureMap& texture) {
  float x = (1 - point.u - point.v) * point.intersectedTriangle.texturePoints[0].x + point.u * point.intersectedTriangle.texturePoints[1].x + point.v * point.intersectedTriangle.texturePoints[2].x;
  float y = (1 - point.u - point.v) * point.intersectedTriangle.texturePoints[0].y + point.u * point.intersectedTriangle.texturePoints[1].y + point.v * point.intersectedTriangle.texturePoints[2].y;

  x *= texture.width;
  y *= texture.height;

  float texturePixel = round(x) + round(y) * texture.width;
  return texture.pixels[texturePixel];
}

vec3 colourToNormal(Colour normalMapColour, RayTriangleIntersection point) {
  float x,y,z;
  if (point.intersectedTriangle.normal == vec3(0,0,1)) { //if front face of cube want z to be positive, x and y should be good
    x = 128 - normalMapColour.red;
    y = normalMapColour.green - 128;
    z = normalMapColour.blue - 128;
  } else if (point.intersectedTriangle.normal == vec3(1,0,0)) { //if right face of cube want z of colour -> x, x-> -z, y -> y
    x = normalMapColour.blue - 128;
    y = normalMapColour.green - 128;
    z = normalMapColour.red - 128;
  } else if (point.intersectedTriangle.normal == vec3(0, 0, -1)) { //if back face of cube inverse x, y is normal, inverse z
    x = normalMapColour.red - 128;
    y = normalMapColour.green - 128;
    z = 128 - normalMapColour.blue;
  } else if (point.intersectedTriangle.normal == vec3(-1, 0, 0)) {
    x = 128 - normalMapColour.blue;
    y = normalMapColour.green - 128;
    z = 128 - normalMapColour.red;
  }

  return normalize(vec3(x, y, z));
}

float normalMapIntensity(RayTriangleIntersection point, vec3 pointNormal, vec3 light) {
  vec3 AoIlightRay = normalize(vec3(light - point.intersectionPoint));
  float AoIintensity = dot(AoIlightRay, pointNormal);
  if(AoIintensity > 1) AoIintensity = 1;
  if (AoIintensity < 0) AoIintensity = 0;
  return AoIintensity;

  float distance = length(light - point.intersectionPoint);
  float proxIntensity = 20 / (4 * PI * distance * distance); //20 = light strength
  if (proxIntensity > 1) proxIntensity = 1;
  if (proxIntensity < 0) proxIntensity = 0;

  //Specular lighting for each vertex
  vec3 specLightRay = normalize(vec3(point.intersectionPoint - light));
  vec3 reflectionRay = normalize(specLightRay - 2 * pointNormal * dot(specLightRay, pointNormal));
  vec3 viewRay = normalize(cameraPosition - point.intersectionPoint);
  float specIntensity = pow(dot(viewRay, reflectionRay), 256);
  if (specIntensity > 1) specIntensity = 1;
  if (specIntensity < 0) specIntensity = 0;

  float combinedIntensity = 0.7 * proxIntensity + 0.5 * AoIintensity + 0.3 * specIntensity;
  if (combinedIntensity < 0.2) combinedIntensity = 0.2;
  if (combinedIntensity > 1) combinedIntensity = 1;
}

int checkShadow(RayTriangleIntersection intersection, vector<vec3> lights, vector<ModelTriangle> modelTriangles) {
  int blockedLights = 0;
  for (int j = 1; j < lights.size(); j++) {
    vec3 ray = lights[j] - intersection.intersectionPoint;
    bool lightBlocked = false;
    for (int i = 0; i < modelTriangles.size() && !lightBlocked; i++) {
      ModelTriangle triangle = modelTriangles[i];
      vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
      vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
      vec3 SPVector = intersection.intersectionPoint - triangle.vertices[0];
      mat3 DEMatrix(normalize(-ray), e0, e1);
      vec3 possibleSolution = inverse(DEMatrix) * SPVector;
      float t = possibleSolution.x;
      float u = possibleSolution.y;
      float v = possibleSolution.z;

      if (u >= 0.0 && u <= 1.0 && v >= 0.0 && v <= 1.0 && u + v <= 1.0) {
        if (t < length(ray) && t > 0.01 && intersection.triangleIndex != i) {
          lightBlocked = true;
          blockedLights++;
        }
      }
    }
  }
  return blockedLights;
}

uint32_t getEnvMapPixelColour(float xRatio, float yRatio, const TextureMap& texture) {
  float x = round(xRatio * (texture.width - 1));
  float y = round(yRatio * (texture.height - 1));

  float texturePixel = x + y * texture.width;
  return texture.pixels[texturePixel];
}

Colour envMapDirection(vec3 ray, unordered_map<string, TextureMap>& TextureMaps) {
  uint32_t envPixelColour;
  vec3 rayDirection = normalize(ray);
  float xRatio;
  float yRatio;
  float largestValue = max(fabs(rayDirection.x), fabs(rayDirection.y), fabs(rayDirection.z));

  if (largestValue == fabs(rayDirection.x)) { // ray will hit left or right wall
    if (rayDirection.x > 0) { // ray will hit right wall
      xRatio = (rayDirection.z / abs(rayDirection.x) + 1 )/ 2;
      yRatio = (-rayDirection.y / abs(rayDirection.x) + 1) / 2;
      envPixelColour = getEnvMapPixelColour(xRatio, yRatio, TextureMaps["../models/env-map/px.ppm"]);
    } else { //ray will hit left wall
      xRatio = (-rayDirection.z / abs(rayDirection.x) + 1 )/ 2;
      yRatio = (-rayDirection.y / abs(rayDirection.x) + 1) / 2;
      envPixelColour = getEnvMapPixelColour(xRatio, yRatio, TextureMaps["../models/env-map/nx.ppm"]);
    }
  } else if (largestValue == fabs(rayDirection.y)) { //ray will hit top or bottom wall
    if (rayDirection.y > 0) { // ray will hit top wall
      xRatio = (rayDirection.x / abs(rayDirection.y) + 1 )/ 2;
      yRatio = (-rayDirection.z / abs(rayDirection.y) + 1) / 2;
      envPixelColour = getEnvMapPixelColour(xRatio, yRatio, TextureMaps["../models/env-map/py.ppm"]);
    } else { //ray will hit bottom wall (correct orientation)
      xRatio = (rayDirection.x / abs(rayDirection.y) + 1 )/ 2;
      yRatio = (rayDirection.z / abs(rayDirection.y) + 1) / 2;
      envPixelColour = getEnvMapPixelColour(xRatio, yRatio, TextureMaps["../models/env-map/ny.ppm"]);
    }
  } else if (largestValue == fabs(rayDirection.z)) { //ray will hit front or back wall
    if (rayDirection.z > 0) { // ray will hit back wall
      xRatio = (-rayDirection.x / abs(rayDirection.z) + 1 )/ 2;
      yRatio = (-rayDirection.y / abs(rayDirection.z) + 1) / 2;
      envPixelColour = getEnvMapPixelColour(xRatio, yRatio, TextureMaps["../models/env-map/nz.ppm"]);
    } else { //ray will hit front wall (correct orientation)
      xRatio = (rayDirection.x / abs(rayDirection.z) + 1 )/ 2;
      yRatio = (-rayDirection.y / abs(rayDirection.z) + 1) / 2;
      envPixelColour = getEnvMapPixelColour(xRatio, yRatio, TextureMaps["../models/env-map/pz.ppm"]);
    }
  }
  return convert_colour_type(envPixelColour);
}


RayTriangleIntersection reflectionGetClosestIntersection(vec3 rayDirection, vector<ModelTriangle> modelTriangles, RayTriangleIntersection intersection, unordered_map<string, TextureMap>& TextureMaps) {
  RayTriangleIntersection rayIntersection;
  rayIntersection.hit = false;
  rayIntersection.distanceFromCamera = numeric_limits<float>::infinity();
  for (int i = 0; i < modelTriangles.size(); i++) {
    ModelTriangle triangle = modelTriangles[i];
    vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
    vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
    vec3 SPVector = intersection.intersectionPoint - triangle.vertices[0];
    mat3 DEMatrix(-rayDirection, e0, e1);
    vec3 possibleSolution = inverse(DEMatrix) * SPVector;
    float t = possibleSolution.x;
    float u = possibleSolution.y;
    float v = possibleSolution.z;

    if (u >= 0.0 && u <= 1.0 && v >= 0.0 && v <= 1.0 && u + v <= 1.0) {
      if (t < rayIntersection.distanceFromCamera && t > 0.00001) {
        rayIntersection.hit = true;
        rayIntersection.distanceFromCamera = t;
        rayIntersection.triangleIndex = i;
        rayIntersection.intersectedTriangle = triangle;
        rayIntersection.intersectionPoint = triangle.vertices[0] + u * e0 + v * e1;
        rayIntersection.u = u;
        rayIntersection.v = v;
        if (rayIntersection.intersectedTriangle.mirror) {
          vec3 surfaceNormal = rayIntersection.intersectedTriangle.normal;
          vec3 reflectionRay = rayDirection - 2 * surfaceNormal * dot(rayDirection, surfaceNormal);
          rayIntersection = reflectionGetClosestIntersection(normalize(reflectionRay), modelTriangles, rayIntersection, TextureMaps);
        } else if (rayIntersection.intersectedTriangle.texture) {
          uint32_t c = texturePixel(rayIntersection, TextureMaps[rayIntersection.intersectedTriangle.colour.name]);
          rayIntersection.pointColour = convert_colour_type(c);
          rayIntersection.u = u;
          rayIntersection.v = v;
          rayIntersection.distanceFromCamera = t;
          rayIntersection.triangleIndex = i;
          rayIntersection.intersectedTriangle = triangle;
        } else {
          rayIntersection.distanceFromCamera = t;
          rayIntersection.triangleIndex = i;
          rayIntersection.intersectedTriangle = triangle;
          rayIntersection.intersectionPoint = triangle.vertices[0] + u * e0 + v * e1;
          rayIntersection.u = u;
          rayIntersection.v = v;
          rayIntersection.pointColour = rayIntersection.intersectedTriangle.colour;
        }
      }
    }
  }
  return rayIntersection;
}

RayTriangleIntersection getClosestIntersection(vec3 rayDirection, vector<ModelTriangle> modelTriangles, unordered_map<string, TextureMap>& TextureMaps) {
  RayTriangleIntersection rayIntersection;
  rayIntersection.hit = false;
  rayIntersection.distanceFromCamera = numeric_limits<float>::infinity();
  for (int i = 0; i < modelTriangles.size(); i++) {
    ModelTriangle triangle = modelTriangles[i];
    vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
    vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
    vec3 SPVector = cameraPosition - triangle.vertices[0];
    mat3 DEMatrix(-rayDirection, e0, e1);
    vec3 possibleSolution = inverse(DEMatrix) * SPVector;
    float t = possibleSolution.x;
    float u = possibleSolution.y;
    float v = possibleSolution.z;
    if (u >= 0.0 && u <= 1.0 && v >= 0.0 && v <= 1.0 && u + v <= 1.0) {
      if (t < rayIntersection.distanceFromCamera && t > 0) {
        rayIntersection.hit = true;
        rayIntersection.distanceFromCamera = t;
        rayIntersection.triangleIndex = i;
        rayIntersection.intersectedTriangle = triangle;
        rayIntersection.intersectionPoint = triangle.vertices[0] + u * e0 + v * e1;
        rayIntersection.u = u;
        rayIntersection.v = v;
        if (rayIntersection.intersectedTriangle.mirror) {
          vec3 surfaceNormal = rayIntersection.intersectedTriangle.normal;
          vec3 reflectionRay = rayDirection - 2 * surfaceNormal * dot(rayDirection, surfaceNormal);
          RayTriangleIntersection reflectionIntersection = reflectionGetClosestIntersection(normalize(reflectionRay), modelTriangles, rayIntersection, TextureMaps);
          if (reflectionIntersection.hit == false) {
            rayIntersection.pointColour = envMapDirection(reflectionRay, TextureMaps);
          } else {
            rayIntersection = reflectionIntersection;
            rayIntersection.distanceFromCamera = t;
          }
        } else if (rayIntersection.intersectedTriangle.texture) {
          uint32_t c = texturePixel(rayIntersection, TextureMaps[rayIntersection.intersectedTriangle.colour.name]);
          rayIntersection.pointColour = convert_colour_type(c);
          rayIntersection.u = u;
          rayIntersection.v = v;
          rayIntersection.distanceFromCamera = t;
          rayIntersection.triangleIndex = i;
          rayIntersection.intersectedTriangle = triangle;
        } else {
          rayIntersection.distanceFromCamera = t;
          rayIntersection.triangleIndex = i;
          rayIntersection.intersectedTriangle = triangle;
          rayIntersection.intersectionPoint = triangle.vertices[0] + u * e0 + v * e1;
          rayIntersection.u = u;
          rayIntersection.v = v;
          rayIntersection.pointColour = rayIntersection.intersectedTriangle.colour;
        }
      }
    }
  }
  return rayIntersection;
}

void rayTraceRender(float focalLength, DrawingWindow &window, vector<ModelTriangle> modelTriangles, unordered_map<string, TextureMap>& TextureMaps, vector<vec3> lights) {
  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < HEIGHT; y++) {
      float xT = x - WIDTH / 2;
      float yT = HEIGHT / 2 - y;
      vec3 transposedPoint = vec3(xT / (WIDTH / 2) , yT / (HEIGHT / 2), -focalLength);
      vec3 rayDirection = normalize(cameraOrientation * transposedPoint);
      RayTriangleIntersection closestIntersection = getClosestIntersection(rayDirection, modelTriangles, TextureMaps);
      int blockedLights = 0;
      if (closestIntersection.hit) blockedLights = checkShadow(closestIntersection, lights, modelTriangles);
      float shadowIntensity = 1;
      if (closestIntersection.intersectedTriangle.shadows && !closestIntersection.intersectedTriangle.normalMap && blockedLights > 0) { //if the point has shadows enabled and at least 1 light point is blocked
        shadowIntensity = 0.8f * (1.0f - static_cast<float>(blockedLights) / (static_cast<float>(lights.size()) - 1.0f)) + 0.2f;
      }
      if (closestIntersection.hit == false) { //if we hit the env map
        Colour envMapColour = envMapDirection(rayDirection, TextureMaps);
        window.setPixelColour(x, y, pack_colour(envMapColour, 1));
      } else if (closestIntersection.intersectedTriangle.normalMap) {
        vec3 pointNormal = colourToNormal(closestIntersection.pointColour, closestIntersection);
        float mapNormalIntensity = normalMapIntensity(closestIntersection, pointNormal, lights[0]);
        window.setPixelColour(x, y, pack_colour(Colour(170, 74, 68), mapNormalIntensity));
      } else {
        if (closestIntersection.intersectedTriangle.shadingMode == PHONG) {
          float intensity = phong(closestIntersection, lights[0]);
          Colour colour = convert_colour_type(pack_colour(closestIntersection.pointColour, intensity));
          window.setPixelColour(x, y, pack_colour(colour, shadowIntensity));
        } else if (closestIntersection.intersectedTriangle.shadingMode == GOURAD) {
          float intensity = gourad(closestIntersection, lights[0]);
          Colour colour = convert_colour_type(pack_colour(closestIntersection.pointColour, intensity));
          window.setPixelColour(x, y, pack_colour(colour, shadowIntensity));
        } else if (closestIntersection.intersectedTriangle.shadingMode == NONE) {
          float intensity = combinedLighting(closestIntersection, lights[0]);
          Colour colour = convert_colour_type(pack_colour(closestIntersection.pointColour, intensity));
          window.setPixelColour(x, y, pack_colour(colour, shadowIntensity));
        }
      }
    }
  }
}

vector<vec3> generateLights(int gridSize, float spacing, vec3 centreLight) {
  vector<vec3> lights;
  lights.push_back(centreLight);
  for (int x = 0; x < gridSize; x++) {
    for (int z = 0; z < gridSize; z++) {
      vec3 light = vec3(
        centreLight.x + (static_cast<float>(x) - static_cast<float>(gridSize) / 2.00f) * spacing,
        centreLight.y,
        centreLight.z + (static_cast<float>(z) - static_cast<float>(gridSize) / 2.00f) * spacing
      );
      lights.push_back(light);
    }
  }
  return lights;
}

void draw(DrawingWindow &window) {
  window.clearPixels();
  orbit(orbiting);
}


void handleEvent(SDL_Event event, DrawingWindow &window, vector<vec3> &lights) {
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
    else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
    else if (event.key.keysym.sym == SDLK_UP) cameraPosition.y += 0.1;
    else if (event.key.keysym.sym == SDLK_DOWN) cameraPosition.y -= 0.1;
    else if (event.key.keysym.sym == SDLK_w) cameraPosition.z -= 0.1;
    else if (event.key.keysym.sym == SDLK_s) cameraPosition.z += 0.1;
    else if (event.key.keysym.sym == SDLK_a) cameraPosition.x -= 0.1;
    else if (event.key.keysym.sym == SDLK_d) cameraPosition.x += 0.1;
    else if (event.key.keysym.sym == SDLK_r) reset_camera();
    else if (event.key.keysym.sym == SDLK_1) cameraPosition = rot_y_axis(-PI/180) * cameraPosition;
    else if (event.key.keysym.sym == SDLK_2) cameraPosition = rot_y_axis(PI/180) * cameraPosition;
    else if (event.key.keysym.sym == SDLK_3) cameraPosition = rot_x_axis(-PI/180) * cameraPosition;
    else if (event.key.keysym.sym == SDLK_4) cameraPosition = rot_x_axis(PI/180) * cameraPosition;
    else if (event.key.keysym.sym == SDLK_q) cameraOrientation = rot_y_axis(PI/180) * cameraOrientation;
    else if (event.key.keysym.sym == SDLK_e) cameraOrientation = rot_y_axis(-PI/180) * cameraOrientation;
    else if (event.key.keysym.sym == SDLK_z) cameraOrientation = rot_x_axis(PI/180) * cameraOrientation;
    else if (event.key.keysym.sym == SDLK_x) cameraOrientation = rot_x_axis(-PI/180) * cameraOrientation;
    else if (event.key.keysym.sym == SDLK_o) orbiting = !orbiting;
    else if (event.key.keysym.sym == SDLK_b) { renderMode = 0; cout << "RenderMode: Wireframe" << endl; }
    else if (event.key.keysym.sym == SDLK_n) { renderMode = 1; cout << "RenderMode: Rasterised" << endl; }
    else if (event.key.keysym.sym == SDLK_m) { renderMode = 2; cout << "RenderMode: Ray Tracing" << endl; }
    else if (event.key.keysym.sym == SDLK_y) { lights[0] = vec3(0, 1.2, -1); }
    else if (event.key.keysym.sym == SDLK_u) { lights[0] = vec3(0, 1.2, 0); }
    else if (event.key.keysym.sym == SDLK_i) { lights[0] = vec3(1, 1.2, 0); }
  } else if (event.type == SDL_MOUSEBUTTONDOWN) {
    window.savePPM("output.ppm");
    window.saveBMP("output.bmp");
  }
}


int main(int argc, char *argv[])
{
  textureToggle = true; //toggle on and off texture mapping
  unordered_map<string, TextureMap> textures;
  if (textureToggle) { //if textures are on map them to the textureMap objects
    textures["../models/texture.ppm"] = TextureMap("../models/texture.ppm");
    textures["../models/brick_normal_map.ppm"] = TextureMap("../models/brick_normal_map.ppm");
    textures["../models/WoodTexture.ppm"] = TextureMap("../models/WoodTexture.ppm");
    // textures["../models/env-map/nx.ppm"] = TextureMap("../models/env-map/left.ppm");
    // textures["../models/env-map/ny.ppm"] = TextureMap("../models/env-map/bottom.ppm");
    // textures["../models/env-map/nz.ppm"] = TextureMap("../models/env-map/back.ppm");
    // textures["../models/env-map/px.ppm"] = TextureMap("../models/env-map/right.ppm");
    // textures["../models/env-map/py.ppm"] = TextureMap("../models/env-map/top.ppm");
    // textures["../models/env-map/pz.ppm"] = TextureMap("../models/env-map/front.ppm");
    textures["../models/env-map/nx.ppm"] = TextureMap("../models/env-map/negx.ppm");
    textures["../models/env-map/ny.ppm"] = TextureMap("../models/env-map/negy.ppm");
    textures["../models/env-map/nz.ppm"] = TextureMap("../models/env-map/negz.ppm");
    textures["../models/env-map/px.ppm"] = TextureMap("../models/env-map/posx.ppm");
    textures["../models/env-map/py.ppm"] = TextureMap("../models/env-map/posy.ppm");
    textures["../models/env-map/pz.ppm"] = TextureMap("../models/env-map/posz.ppm");
  }

  //soft shadow lights initialisation
  bool softShadows = false;
  float spacing = 0.035;
  int gridSize = 16.00;
  vec3 centrelight(0,1.3,0);
  vector<vec3> lights;
  if (softShadows) {
    lights = generateLights(gridSize, spacing, centrelight);
  } else {
    lights.push_back(centrelight);
  }

  vector<ModelTriangle> modelTriangles;
  reset_camera();
  if (textureToggle) {
    modelTriangles = parseObj("../models/textured-cornell-box.obj", 0.3, parseMtl("../models/textured-cornell-box.mtl"), vec3(0,0,0), true, NONE);
  } else {
    modelTriangles = parseObj("../models/cornell-box.obj", 0.3, parseMtl("../models/cornell-box.mtl"), vec3(0,0,0), true, NONE);
  }
  vector<ModelTriangle> normalCubeTriangles = parseObj("../models/normal_map_cube.obj", 0.2, parseMtl("../models/normal_map_cube.mtl"), vec3(-0.62, -0.62, 0.55), true, NONE);
  vector<ModelTriangle> woodTopTriangles = parseObj("../models/wood-top.obj", 0.2, parseMtl("../models/wood-top.mtl"), vec3(-0.62, -0.62, 0.55), true, NONE);
  // vector<ModelTriangle> lightCubeTriangles = parseObj("../models/light_cube.obj", 0.1, parseMtl("../models/normal_map_cube.mtl"), lights[0], false, NONE);
  // vector<ModelTriangle> sphereTriangles = parseObj("../models/sphere.obj", 0.35, parseMtl("../models/sphere.mtl"), vec3(1,-1,-1.5), false, PHONG);
  // modelTriangles.insert(modelTriangles.end(), sphereTriangles.begin(), sphereTriangles.end());
  modelTriangles.insert(modelTriangles.end(), normalCubeTriangles.begin(), normalCubeTriangles.end());
  modelTriangles.insert(modelTriangles.end(), woodTopTriangles.begin(), woodTopTriangles.end());
  // modelTriangles.insert(modelTriangles.end(), lightCubeTriangles.begin(), lightCubeTriangles.end());
  DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
  SDL_Event event;
  while (true) {
    // We MUST poll for events - otherwise the window will freeze !
    if (window.pollForInputEvents(event)) handleEvent(event, window, lights);
    draw(window);
    if (renderMode == 0) { wireFrameRender(2.0, window, modelTriangles); }
    else if (renderMode == 1) { rasterisedRender(2.0, window, modelTriangles); }
    else if (renderMode == 2) { rayTraceRender(2.0, window, modelTriangles, textures, lights); }
    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }
}