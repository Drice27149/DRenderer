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

// data to build Pipeline state object
struct PSOData {
    // for init pso
    bool enableDepth = true;
};

// data to ref resource
struct ResourceData {
    std::string name;
    ResourceEnum::State state;
    ResourceEnum::Usage usage;
    ResourceEnum::Type type;
    ResourceEnum::Format format;
};

struct ResourceView {
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpu[ResourceEnum::View::UKnownView];
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpu[ResourceEnum::View::UKnownView];
    unsigned int mask = 0;
};

struct PassData {
    std::vector<ResourceData> inputs;
    std::vector<ResourceData> outputs;
    std::vector<ShaderData> shaders;
    PSOData psoData;
};
