#ifndef MTL_PARSER_H
#define MTL_PARSER_H

#include <string>
#include <unordered_map>
#include "Colour.h"

std::unordered_map<std::string, Colour> parseMtl(std::string filename);

#endif