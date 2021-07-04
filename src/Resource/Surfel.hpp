#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "Reflect.hpp"

using namespace glm;

class Surfel: public Reflectable{
public:
    vec3 position;
    vec3 normal;
    vec3 albedo;
public:
    virtual std::vector<Reflect::Data> serialize() override
    {
        std::vector<Reflect::Data> res = {
            // offset, type, name[3], data[3]
            Reflect::Data{offsetof(Surfel, position), Reflect::Type::FLOAT3, "PX", "PY", "PZ", position[0], position[1], position[2]},
            Reflect::Data{offsetof(Surfel, normal), Reflect::Type::FLOAT3, "NX", "NY", "NZ", normal[0], normal[1], normal[2]},
            Reflect::Data{offsetof(Surfel, albedo), Reflect::Type::FLOAT3, "CX", "CY", "CZ", albedo[0], albedo[1], albedo[2]},
        };
	    return res;
    }
};

class SurfelCluster {   
public:
    int offset;
    int count;
};