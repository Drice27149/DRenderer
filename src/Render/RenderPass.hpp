#pragma once

#include "RenderStruct.hpp"
#include "d3d12.h"
#include "d3dUtil.h"

using namespace Microsoft::WRL;

class RenderPass {
public:
    PassData& GetPassData();
public:
    PassData data;
    ComPtr<ID3D12PipelineState> pso;
    ComPtr<ID3D12RootSignature> rts;
};