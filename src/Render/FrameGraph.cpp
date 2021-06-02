#include "FrameGraph.hpp"
#include "d3dUtil.h"
#include "RenderStruct.hpp"
#include "Graphics.hpp"
#include "Device.hpp"

template<typename Setup, typename Execute>
void FrameGraph::AddPass(std::string name, Setup setup, Execute exe){
    // frontend set up, read back pso data
    RenderPass newPass;
    setup(newPass.GetPassData());
    // backend setup, create root signature and pso
    device->SetUpRenderPass(newPass, newPass.GetPassData());
    passes.emplace_back(newPass);
/**delay execution**/
    // backend exe, set pso, root signature, root params, rendertargets 
    device->ExecuteRenderPass(newPass, newPass.GetPassData());
    // frontend exe
    exe();
}