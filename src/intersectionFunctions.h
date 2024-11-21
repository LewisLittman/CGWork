//
// Created by ok22037 on 21/11/24.
//

#ifndef INTERSECTIONFUNCTIONS_H
#define INTERSECTIONFUNCTIONS_H

#endif //INTERSECTIONFUNCTIONS_H
RayTriangleIntersection reflectionGetClosestIntersection(vec3 rayDirection, std::vector<ModelTriangle> modelTriangles,
    vec3 rayOrigin, std::unordered_map<std::string, TextureMap>& TextureMaps, int recursionCount);