#include "ObjParser.h"
#include <fstream>
#include <Utils.h>
#include <string>

std::vector<ModelTriangle> parseObj(std::string filename, float scale, std::unordered_map<std::string, Colour> colours, glm::vec3 offset, bool shadows, int shadingMode) {
  std::ifstream File(filename);
  std::string line;
  std::vector<ModelTriangle> triangles;
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> vertexNormals;
  std::vector<TexturePoint> texturePoints;
  std::string colour;
  std::string name;
  while (std::getline(File, line)) {
      if (line.empty()) continue;
      std::vector<std::string> values = split(line, ' ');
      if (values[0] == "o") {
          name = values[1];
      } else if (values[0] == "v") {
          glm::vec3 vertex(stof(values[1]) * scale, stof(values[2]) * scale, stof(values[3]) * scale);
          vertex += offset;
          vertices.push_back(vertex);
          vertexNormals.emplace_back(0.0f, 0.0f, 0.0f);
      } else if (values[0] == "vt") {
          texturePoints.push_back(TexturePoint(stof(values[1]), stof(values[2])));
      } else if (values[0] == "f") {
          int i1 = std::stoi(values[1]) - 1;
          int i2 = std::stoi(values[2]) - 1;
          int i3 = std::stoi(values[3]) - 1;
          std::vector<std::string> v1 = split(values[1], '/');
          std::vector<std::string> v2 = split(values[2], '/');
          std::vector<std::string> v3 = split(values[3], '/');
          ModelTriangle triangle(vertices[stoi(v1[0]) - 1], vertices[stoi(v2[0]) - 1], vertices[stoi(v3[0]) - 1], colours[colour]);
          triangle.shadows = shadows;
          triangle.shadingMode = shadingMode;
          if (colour == "Mirror") {
            triangle.mirror = true;
            std::cout << colour << std::endl;
          }
          if (name == "light") {
            triangle.name = name;
          }
          if (colour == "Metal") {
            triangle.metal = true;
          }
          if (colour == "NormalMap") {
            triangle.normalMap = true;
            triangle.texture = true;
          }
          if (colour == "Texture") {
            triangle.texture = true;
          }
          if (v1.size() > 1 && !v1[1].empty() && v2.size() > 1 && !v2[1].empty() && v3.size() > 1 && !v3[1].empty()) {
            triangle.texturePoints[0] = texturePoints[stoi(v1[1]) - 1];
            triangle.texturePoints[1] = texturePoints[stoi(v2[1]) - 1];
            triangle.texturePoints[2] = texturePoints[stoi(v3[1]) - 1];
          }
          triangles.push_back(triangle);
          // Calculate face normal and add it to vertex normals
          glm::vec3 faceNormal = normalize(cross(vertices[i1] - vertices[i3], vertices[i2] - vertices[i3]));
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