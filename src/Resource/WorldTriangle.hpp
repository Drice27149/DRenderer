#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

class WorldTriangle {
public:
    vec3 position[3];
    vec3 normal;
    float area;
public:
    WorldTriangle(vec3 v0, vec3 v1, vec3 v2)
    {
        position[0] = v0;
        position[1] = v1;
        position[2] = v2;
        normal = glm::cross(v1-v0, v2-v0);
        area = Area();
    }

    float Area()
    {
        vec3 ab = position[1] - position[0];
        vec3 ac = position[2] - position[0];
        float area = glm::length(glm::cross(ab, ac)) * 0.5;
        return area;
    }
};