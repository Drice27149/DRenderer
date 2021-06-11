#pragma once

#include <iostream>
#include <vector>
#include "RenderEnum.hpp"
#include "d3d12.h"
#include "d3dUtil.h"

using namespace Microsoft::WRL;

// data to compile shader
struct ShaderData {
    std::string name;
    ShaderEnum::Type type;
};

// data to ref resource
struct ResourceData {
    std::string name;
    ResourceEnum::State state;      // read / write / create
    ResourceEnum::Type type;        // constant, buffer, texture2D, textureCube
    ResourceEnum::Format format;    // R32G32B32A32_FLOAT
    //ResourceEnum::Usage usage;      // not using
};

struct ResourceView {
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpu[ResourceEnum::View::UKnownView];
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpu[ResourceEnum::View::UKnownView];
    unsigned int mask = 0;
};

struct ResourceDesc {
    unsigned int width;
    unsigned int height;
    ResourceEnum::Format format;
    ResourceEnum::Type type;
    ResourceEnum::State state;
};

// data to build Pipeline state object
struct PSOData {
    // for init pso
    bool enableDepth = true;
    // for deferred shading blend op add
    bool enableAdd = false;
    // PSOEnum::BlendMode blendMode = @TODO: blend op for transparent obj ?
    ResourceData depthStencil;
    int width = -1;
    int height = -1;
};

struct PassData {
    std::vector<ResourceData> inputs;
    std::vector<ResourceData> outputs;
    std::vector<ShaderData> shaders;
    PSOData psoData;
};
