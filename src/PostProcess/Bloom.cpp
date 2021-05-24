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
    for (int i = 0; i < 3; i++) {
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
    vs = d3dUtil::CompileShader(L"..\\assets\\shaders\\AA\\filter.hlsl", nullptr, "VS", "vs_5_1");
    ps = d3dUtil::CompileShader(L"..\\assets\\shaders\\AA\\filter.hlsl", nullptr, "PS", "ps_5_1");
    xvs = d3dUtil::CompileShader(L"..\\assets\\shaders\\AA\\downSample.hlsl", nullptr, "VS", "vs_5_1");
    xps = d3dUtil::CompileShader(L"..\\assets\\shaders\\AA\\downSample.hlsl", nullptr, "PS", "ps_5_1");
    yvs = d3dUtil::CompileShader(L"..\\assets\\shaders\\AA\\downSample0.hlsl", nullptr, "VS", "vs_5_1");
    yps = d3dUtil::CompileShader(L"..\\assets\\shaders\\AA\\downSample0.hlsl", nullptr, "PS", "ps_5_1");
}

void Bloom::PrePass()
{
}

void Bloom::PostPass()
{

}

void Bloom::BloomPass()
{
    Graphics::GCmdList->SetPipelineState(pso.Get());
    Graphics::GCmdList->SetGraphicsRootSignature(rootSig.Get());
    Graphics::GCmdList->SetGraphicsRootConstantBufferView(0, Graphics::constantMgr->GetSceneInfoConstant());
    Graphics::GCmdList->SetGraphicsRootDescriptorTable(1, input);

    BloomLoop(0);

    Graphics::GCmdList->SetGraphicsRootSignature(rootSig.Get());
    Graphics::GCmdList->SetGraphicsRootConstantBufferView(0, Graphics::constantMgr->GetSceneInfoConstant());
    Graphics::GCmdList->SetGraphicsRootDescriptorTable(1, srvGpu[0]);

    for (int i = 1; i < 3; i++) {
        if(i%2) Graphics::GCmdList->SetPipelineState(xpso.Get());
        else Graphics::GCmdList->SetPipelineState(ypso.Get());
        BloomLoop(i);
    }
}

void Bloom::BloomLoop(unsigned int i)
{
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

void Bloom::BuildPSO()
{
    PostProcess::BuildPSO();
    BuildBloomPSO(xvs, xps, xpso);
    BuildBloomPSO(yvs, yps, ypso);
}

void Bloom::BuildBloomPSO(ComPtr<ID3DBlob>& newVs, ComPtr<ID3DBlob>& newPs, ComPtr<ID3D12PipelineState>& newpso)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = { inputLayout.data(), (unsigned int)inputLayout.size() };
    psoDesc.pRootSignature = rootSig.Get();

    D3D12_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = false;

    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.FrontCounterClockwise = true;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = depthDesc;// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    // build extra pso for grid
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.VS =
    {
        reinterpret_cast<BYTE*>(newVs->GetBufferPointer()),
        newVs->GetBufferSize()
    };
    psoDesc.GS = { nullptr, 0 };
    psoDesc.PS =
    {
        reinterpret_cast<BYTE*>(newPs->GetBufferPointer()),
        newPs->GetBufferSize()
    };
    ThrowIfFailed(Graphics::GDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&newpso)));
}