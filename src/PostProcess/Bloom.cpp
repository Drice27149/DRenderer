#include "Bloom.hpp"
#include "Graphics.hpp"

void Bloom::Init()
{
    CompileShaders();
    CreateResources();

    BuildRootSig();
    BuildPSO();
}

void Bloom::CreateResources()
{
    // @TODO: resource manager, after post process done and color grading with filament done
    unsigned int width = Graphics::viewPortWidth;
    unsigned int height = Graphics::viewPortHeight;
    for (int i = 0; i < 4; i++) {
        Graphics::heapMgr->GetNewRTV(rtvCpu[i], rtvGpu[i]);
        Graphics::heapMgr->GetNewSRV(srvCpu[i], srvGpu[i]);
        downSampleRT[i] = std::make_unique<Resource>(Graphics::GDevice, Graphics::GCmdList);
        downSampleRT[i]->BuildRenderTarget(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
        downSampleRT[i]->AppendTexture2DRTV(DXGI_FORMAT_R32G32B32A32_FLOAT, rtvCpu[i]);
        downSampleRT[i]->AppendTexture2DSRV(DXGI_FORMAT_R32G32B32A32_FLOAT, srvCpu[i]);
    }
}

void Bloom::CompileShaders()
{
    vs = d3dUtil::CompileShader(L"..\\assets\\shaders\\AA\\downSample.hlsl", nullptr, "VS", "vs_5_1");
    ps = d3dUtil::CompileShader(L"..\\assets\\shaders\\AA\\downSample.hlsl", nullptr, "PS", "ps_5_1");
}

void Bloom::PrePass()
{
    Graphics::GCmdList->SetPipelineState(pso.Get());
    Graphics::GCmdList->SetGraphicsRootSignature(rootSig.Get());
    Graphics::GCmdList->SetGraphicsRootConstantBufferView(0, Graphics::constantMgr->GetSceneInfoConstant());
}

void Bloom::PostPass()
{

}

void Bloom::BloomPass()
{
    this->PrePass();
    Graphics::GCmdList->SetGraphicsRootDescriptorTable(1, input);
    for (int i = 0; i < 4; i++) {
        Graphics::GCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                downSampleRT[i]->mResource.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_RENDER_TARGET
            )
        );
        Graphics::GCmdList->OMSetRenderTargets(1, &(rtvCpu[i]), false, nullptr);
        Graphics::GCmdList->ClearRenderTargetView(rtvCpu[i], Colors::Black, 0, nullptr);
        this->Pass();
        Graphics::GCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
                downSampleRT[i]->mResource.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
            )
        );
        Graphics::GCmdList->SetGraphicsRootDescriptorTable(1, srvGpu[i]);
    }
}