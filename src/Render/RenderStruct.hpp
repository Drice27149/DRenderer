#pragma once

#include <iostream>
#include <vector>
#include "RenderEnum.hpp"
#include "d3d12.h"
#include "d3dUtil.h"

using namespace Microsoft::WRL;

// data to build root signature
struct RStData {

    ComPtr<ID3D12RootSignature> rst = nullptr;
};

// data to compile shader
struct ShaderData {
    std::string name;
    ShaderType type;
    // for self defined shader
    ComPtr<ID3DBlob> shader = nullptr;
};

// data to build Pipeline state object
struct PSOData {
    //for self defined pso
    ComPtr<ID3D12PipelineState> pso = nullptr;
};

// data to ref resource
struct ResourceData {
    std::string name;
    ResourceState state;
    ViewType view;
};

struct PassData {
    std::vector<ResourceData> inputs;
    std::vector<ResourceData> outputs;
    std::vector<ShaderData> shaders;
    RStData rtsData;
    PSOData psoData;
};