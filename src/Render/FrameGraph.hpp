
#pragma once

#include <iostream>
#include <vector>

class RenderPass;
struct PassData;
struct PSOData;
struct RStData;

class FrameGraph {
public:
    template<typename Setup, typename Execute>
    void AddPass(std::string name, Setup setup, Execute execute);
private:
    std::vector<RenderPass> passes;
};

// some helper function
void BuildPass(RenderPass& pass, PassData& data);
void BuildPSO(RenderPass& pass, PSOData& data);
void BuildRootSignature(RenderPass& pass, RStData& data);
// ComPtr<ID3DBlob> GetShader(ShaderData data)