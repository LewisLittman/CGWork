#include "Scene.h"

void Scene::addTriangles(const std::vector<ModelTriangle>& newTriangles) {
    triangles.insert(triangles.end(), newTriangles.begin(), newTriangles.end());
}
