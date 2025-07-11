#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <unordered_map>
#include <string>
#include "../../libs/sdw/ModelTriangle.h"
#include "Camera.h"
#include "TextureMap.h"

class Scene {
public:
    std::vector<ModelTriangle> triangles;
    std::vector<glm::vec3> lights;
    std::unordered_map<std::string, TextureMap> textures;
    Camera camera;

    Scene() = default;
    void addTriangles(const std::vector<ModelTriangle>& newTriangles);
};

#endif