
#pragma once

#include <iostream>
#include <vector>
#include "RenderPass.hpp"

class Device;
struct PassData;
struct PSOData;
struct RStData;

class FrameGraph {
public:
    /**
     * data.inputs = {ResourceData};
     * data.outputs = {ResourceData};
     * data.shaders = {ShaderData};
     * data.psoData.enableDepth = false;
     */
    template<typename Setup, typename Execute>
    void AddPass(std::string name, Setup setup, Execute execute);
private:
    std::vector<RenderPass> passes;
};


template<typename Setup, typename Execute>
void FrameGraph::AddPass(std::string name, Setup setup, Execute exe){

    RenderPass newPass;
    // frontend set up, read back pso data
    setup(newPass.GetPassData());

    // backend setup, create root signature and pso
    Renderer::GDevice->SetUpRenderPass(newPass, newPass.GetPassData());
    passes.emplace_back(newPass);

/**delay execution**/
    // backend exe, set pso, root signature, root params, rendertargets 
    Renderer::GDevice->ExecuteRenderPass(newPass, newPass.GetPassData());

    // frontend exe
    exe();
}