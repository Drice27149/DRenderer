#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "d3dUtil.h"

struct PassUniform
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 SMView;
    glm::mat4 SMProj;
    glm::mat4 JProj;
    // зЂвт padding
    glm::vec3 CamPos;
};

struct ObjectUniform
{
    glm::mat4 model;
    unsigned int id;
    unsigned int mask;
    float metallic;
    float roughness;
};

struct metaData {
	std::string name;
	unsigned long long offset;
    // 0 for int, 1 for float
    unsigned int type;
	metaData(std::string name, unsigned long long offset, unsigned int type):name(name), offset(offset), type(type){}
};

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

struct SceneInfo {
    // main Light
    float posX;
    float posY;
    float posZ;
    float dirX;
    float dirY;
    float dirZ;
    float lightIntensity;
    float envIntensity;
    int taa;
    float taaAlpha;
    static std::vector<metaData> reflections;
};

// global functions
std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();