#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct LightNode {
    unsigned int id;
    unsigned int next;
};

struct LightInfo {
    unsigned int id;
    glm::vec3 pos;
    float radiance;
};

struct ClusterInfo {
    unsigned int clusterX;
    unsigned int clusterY;
    unsigned int clusterZ;
    float cNear;
    float cFar;
};