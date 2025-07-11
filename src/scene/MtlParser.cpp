#include "MtlParser.h"
#include <fstream>
#include <Utils.h>
#include <string>
#include <unordered_map>

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