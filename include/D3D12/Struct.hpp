#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "d3dUtil.h"

struct ClusterInfo {
    unsigned int clusterX;
    unsigned int clusterY;
    unsigned int clusterZ;
    float cNear;
    float cFar;
};

struct LightOffset {
    unsigned int offset;
};

struct LightEntry {
    unsigned int lightID;
    unsigned int next;
};

struct LightInfo {
    unsigned int id;
    glm::vec3 pos;
};

// global functions
std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();