#ifndef OBJ_PARSER_H
#define OBJ_PARSER_H

#include <string>
#include <vector>
#include <unordered_map>
#include "ModelTriangle.h"

std::vector<ModelTriangle> parseObj(std::string filename, float scale, std::unordered_map<std::string, Colour> colours, glm::vec3 offset, bool shadows, int shadingMode);

#endif