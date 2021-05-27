#include <cmath>
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
    // pipeline state object
    PSOFatory::CreatePostProcessPSO(pso, vs, nullptr, ps, rootSig);
    // prefilter env map
    {
        // create cube map
        ResourceFatory::CreateCubeMapResource(envMap, width, height, mipLevels = 5);
        envMap->SetName(L"envMap");
        // build shader resource view
        Graphics::heapMgr->GetNewSRV(envC, envG);
        DescriptorFatory::AppendCubeSRV(envMap, DXGI_FORMAT_R32G32B32A32_FLOAT, envC);
        // rtv to write 
        for(int i = 0; i < 6; i++){
            for(int j = 0; j < mipLevels; j++){
                // stuff to create rtv
                D3D12_RENDER_TARGET_VIEW_DESC rtvDesc; 
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY; 
                rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                // roughness level at j 
                rtvDesc.Texture2DArray.MipSlice = j; 
                rtvDesc.Texture2DArray.PlaneSlice = 0; 
                rtvDesc.Texture2DArray.FirstArraySlice = i; 
                rtvDesc.Texture2DArray.ArraySize = 1; 
                // create rtv
                unsigned int id = i*mipLevels + j;
                Graphics::heapMgr->GetNewRTV(sRtvC[id], sRtvG[id]);
                Graphics::GDevice->CreateRenderTargetView(envMap.Get(), &rtvDesc, sRtvC[id]);
            }
        }
    }
    // brdf map, load by image
    {
        ResourceFatory::CreateTexture2DResource(brdfMap, uploadBuffer, "../assets/envs/dfg_approx.png");
        brdfMap->SetName(L"brdfMap");
        // build shader resource view
        Graphics::heapMgr->GetNewSRV(brdfC, brdfG);
        DescriptorFatory::AppendTexture2DSRV(brdfMap, brdfC);
    }
    // viewport setting
    long curWidth = width;
    long curHeight = height;
    for(int i = 0; i < mipLevels; i++){
        screenViewport[i].TopLeftX = 0;
        screenViewport[i].TopLeftY = 0;
        screenViewport[i].Width    = static_cast<float>(curWidth);
        screenViewport[i].Height   = static_cast<float>(curHeight);
        screenViewport[i].MinDepth = 0.0f;
        screenViewport[i].MaxDepth = 1.0f;
        scissorRect[i] = { 0, 0, curWidth, curHeight };
        curWidth /= 2;
        curHeight /= 2;
    }

    // pass constants stuff
    passConstant = std::make_unique<UploadBuffer<PassID>>(Graphics::GDevice, 6*mipLevels+6, true);
    // [0, 6*5] specular ibl
    for(int i = 0; i < 6; i++){
        for(int j = 0; j < mipLevels; j++){
            PassID cur;
            cur.id = i;
            cur.roughness = (float)j / (float)(mipLevels-1);
            passConstant->CopyData(i*mipLevels+j, cur);
        }
    }
}

void PrefilterIBL::PreComputeFilter()
{
    Graphics::GCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        envMap.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET
        )
    );

    Graphics::GCmdList->SetPipelineState(pso.Get());
    Graphics::GCmdList->SetGraphicsRootSignature(rootSig.Get());   
    DxUtil::BindGraphicsRoot(rootors);
    // for each face, for each roughness level, do it
    for(int i = 0; i < 6; i++){
        for(int j = 0; j < mipLevels; j++){
            Graphics::GCmdList->OMSetRenderTargets(1, &(sRtvC[i*mipLevels+j]), false, nullptr);
            Graphics::GCmdList->RSSetScissorRects(1, &(scissorRect[j]));
            Graphics::GCmdList->RSSetViewports(1, &(screenViewport[j]));
            Graphics::GCmdList->SetGraphicsRootConstantBufferView(0, GetPassConstantAddr(i*mipLevels+j));
            DxUtil::FullScreenPass();
        }
    }

    Graphics::GCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        envMap.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        )
    );
}

D3D12_GPU_VIRTUAL_ADDRESS PrefilterIBL::GetPassConstantAddr(unsigned long long offset)
{
    auto beginAddr = passConstant->Resource()->GetGPUVirtualAddress();
    beginAddr += offset * d3dUtil::CalcConstantBufferByteSize(sizeof(PassID));
    return beginAddr;
}