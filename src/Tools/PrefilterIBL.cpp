#include "PrefilterIBL.hpp"
#include "DxUtil.hpp"
#include "Fatory.hpp"

void PrefilterIBL::Init()
{
    // compile shader @TODO: shader manager
    vs = d3dUtil::CompileShader(L"..\\assets\\shaders\\precompute\\diffuseFilter.hlsl", nullptr, "VS", "vs_5_1");
	ps = d3dUtil::CompileShader(L"..\\assets\\shaders\\precompute\\diffuseFilter.hlsl", nullptr, "PS", "ps_5_1");
    // build rootsig and pso
    RootSigFatory::CreateRootSig(rootSig, rootors, true);
    PSOFatory::CreatePostProcessPSO(pso, vs, nullptr, ps, rootSig);
    ResourceFatory::CreateCubeMapResource(diffuseMap, width, height);
    diffuseMap->SetName(L"diffuseMap");
    // cube map srv
    Graphics::heapMgr->GetNewSRV(diffuseC, diffuseG);
    DescriptorFatory::AppendCubeSRV(diffuseMap, DXGI_FORMAT_R32G32B32A32_FLOAT, diffuseC);
    // cube map seven rtv
    for(int i = 0; i < 6; i++){
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc; 
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY; 
        rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; 
        rtvDesc.Texture2DArray.MipSlice = 0; 
        rtvDesc.Texture2DArray.PlaneSlice = 0; 
        // 将场景绘制到第i个成员 
        rtvDesc.Texture2DArray.FirstArraySlice = i; 
        rtvDesc.Texture2DArray.ArraySize = 1; 
        Graphics::heapMgr->GetNewRTV(rtvC[i], rtvG[i]);
        // 为cube map的第i个面创建RTV 
        Graphics::GDevice->CreateRenderTargetView(diffuseMap.Get(), &rtvDesc, rtvC[i]); 
    }
    // viewport setting
    screenViewport.TopLeftX = 0;
	screenViewport.TopLeftY = 0;
	screenViewport.Width    = static_cast<float>(width);
	screenViewport.Height   = static_cast<float>(height);
	screenViewport.MinDepth = 0.0f;
	screenViewport.MaxDepth = 1.0f;
	scissorRect = { 0, 0, (long)width, (long)height };
}

void PrefilterIBL::PreComputeFilter()
{
    Graphics::GCmdList->SetPipelineState(pso.Get());
    Graphics::GCmdList->SetGraphicsRootSignature(rootSig.Get());   
    DxUtil::BindGraphicsRoot(rootors);
    Graphics::GCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        diffuseMap.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET
        )
    );
    for(int i = 0; i < 6; i++){
        Graphics::GCmdList->OMSetRenderTargets(1, &(rtvC[i]), false, nullptr);
        Graphics::GCmdList->RSSetScissorRects(1, &scissorRect);
        Graphics::GCmdList->RSSetViewports(1, &screenViewport);
        Graphics::GCmdList->SetGraphicsRootConstantBufferView(0, Graphics::constantMgr->GetPassID(i));
        DxUtil::FullScreenPass();
    }
    Graphics::GCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        diffuseMap.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        )
    );
}