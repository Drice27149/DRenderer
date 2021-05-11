#include "ShadowMgr.hpp"

void ShadowMgr::PrePass()
{
    commandList->OMSetRenderTargets(0, nullptr, false, &depthMap->xxxCpu);
    commandList->RSSetViewports(1, &depthMap->Viewport());
    commandList->RSSetScissorRects(1, &depthMap->ScissorRect());

    commandList->ResourceBarrier(1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(
            depthMap->GetResource(),
            D3D12_RESOURCE_STATE_GENERIC_READ, 
            D3D12_RESOURCE_STATE_DEPTH_WRITE
        )
    );

    commandList->ClearDepthStencilView(depthMap->xxxCpu, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    commandList->SetPipelineState(pso.Get()); 
    commandList->SetGraphicsRootSignature(rootSig.Get());

    auto passAddr = constantMgr->GetShadowPassConstant();
    commandList->SetGraphicsRootConstantBufferView(0, passAddr);
}