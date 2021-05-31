#include "FrameGraph.hpp"
#include "d3dUtil.h"
#include "RenderStruct.hpp"
#include "Graphics.hpp"

template<typename Setup, typename Execute>
void FrameGraph::AddPass(std::string name, Setup setup, Execute exe){
    // execute set up lambda
    RenderPass newPass;
    setup(newPass.GetPassData());
    // build root params, pipeline state object
    FrameGraph::BuildPass(newPass);
/**delay**/
    // set pipeline state object and root signature
    // set root arguments
    // execute execute lambda
}

void BuildPass(RenderPass& pass, PassData& data)
{   
    for(ShaderData& shader: data.shaders){
        // shader.shader = GetShader(shader);
    }
    BuildRootSignature(pass, data.rtsData);
    BuildPSO(pass, data.psoData);
}


// some helper function
void BuildPSO(RenderPass& pass, PSOData& data)
{

}

void BuildRootSignature(RenderPass& pass, RStData& data)
{

}

// ComPtr<ID3DBlob> GetShader(ShaderData data)
// {
//     if(data.shader != nullptr)
//         return data.shader;
//     if(shaders.count(data.name))
//         return shaders[data.name];
//     if(data.type == ShaderType::CS)
//         return d3dUtil::CompileShader(GetWString(data.name), nullptr, "CS", "cs_5_1");
//     else if(data.type == ShaderType::GS)
//         return d3dUtil::CompileShader(GetWString(data.name), nullptr, "GS", "gs_5_1");
//     else if(data.type == ShaderType::PS)
//         return d3dUtil::CompileShader(GetWString(data.name), nullptr, "PS", "ps_5_1");
//     else
//         return d3dUtil::CompileShader(GetWString(data.name), nullptr, "VS", "vs_5_1");
// }

