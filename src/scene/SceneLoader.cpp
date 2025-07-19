#include "SceneLoader.h"
#include "TextureMap.h"
#include "MtlParser.h"
#include "ObjParser.h"
#include "Scene.h"
#include <filesystem>
#include <iostream>

enum ShadingMode {
  NONE = 0,
  GOURAD = 1,
  PHONG = 2
};

Scene SceneLoader::loadScene() {
    Scene scene;

    // Textures
    scene.textures["/home/lewis/CGWork/models/texture.ppm"] = TextureMap("/home/lewis/CGWork/models/texture.ppm");
    scene.textures["/home/lewis/CGWork/models/brick_normal_map.ppm"] = TextureMap("/home/lewis/CGWork/models/brick_normal_map.ppm");
    scene.textures["/home/lewis/CGWork/models/WoodTexture.ppm"] = TextureMap("/home/lewis/CGWork/models/WoodTexture.ppm");
    scene.textures["/home/lewis/CGWork/models/env-map/nx.ppm"] = TextureMap("/home/lewis/CGWork/models/env-map/negx.ppm");
    scene.textures["/home/lewis/CGWork/models/env-map/ny.ppm"] = TextureMap("/home/lewis/CGWork/models/env-map/negy.ppm");
    scene.textures["/home/lewis/CGWork/models/env-map/nz.ppm"] = TextureMap("/home/lewis/CGWork/models/env-map/negz.ppm");
    scene.textures["/home/lewis/CGWork/models/env-map/px.ppm"] = TextureMap("/home/lewis/CGWork/models/env-map/posx.ppm");
    scene.textures["/home/lewis/CGWork/models/env-map/py.ppm"] = TextureMap("/home/lewis/CGWork/models/env-map/posy.ppm");
    scene.textures["/home/lewis/CGWork/models/env-map/pz.ppm"] = TextureMap("/home/lewis/CGWork/models/env-map/posz.ppm");

    // Models
    auto cornellBox = parseObj("/home/lewis/CGWork/models/textured-cornell-box.obj", 0.3f, parseMtl("/home/lewis/CGWork/models/textured-cornell-box.mtl"), glm::vec3(0, 0, 0), true, NONE);
    auto mirror     = parseObj("/home/lewis/CGWork/models/mirror.obj", 0.3f, parseMtl("/home/lewis/CGWork/models/mirror.mtl"), glm::vec3(0, 0, 0), true, NONE);
    auto normalCube = parseObj("/home/lewis/CGWork/models/normal_map_cube.obj", 0.2f, parseMtl("/home/lewis/CGWork/models/normal_map_cube.mtl"), glm::vec3(-0.62, -0.62, 0.55), true, NONE);
    auto woodTop    = parseObj("/home/lewis/CGWork/models/wood-top.obj", 0.2f, parseMtl("/home/lewis/CGWork/models/wood-top.mtl"), glm::vec3(-0.62, -0.62, 0.55), true, NONE);
    auto sphere     = parseObj("/home/lewis/CGWork/models/sphere.obj", 0.3f, parseMtl("/home/lewis/CGWork/models/sphere.mtl"), glm::vec3(-0.5, 0.05, -0.80), false, PHONG);
    auto bunny      = parseObj("/home/lewis/CGWork/models/lpbunny.obj", 0.25f, parseMtl("/home/lewis/CGWork/models/lpbunny.mtl"), glm::vec3(0.05, -0.05, 0.05), false, NONE);

    scene.addTriangles(cornellBox);
    scene.addTriangles(mirror);
    scene.addTriangles(normalCube);
    scene.addTriangles(woodTop);
    scene.addTriangles(sphere);
    scene.addTriangles(bunny);

    scene.lights.push_back(glm::vec3(0, 0.8, 0));

    return scene;
}
