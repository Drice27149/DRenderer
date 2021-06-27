#include "Mipmap.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"
#include "Device.hpp"
#include "Graphics.hpp"
#include "Context.hpp"

CD3DX12_CPU_DESCRIPTOR_HANDLE Mipmap::mipCpu[16];
CD3DX12_GPU_DESCRIPTOR_HANDLE Mipmap::mipGpu[16];

void Mipmap::CreateMipmap(unsigned int length)
{
    int l = length;
    unsigned int count = 0;
    while(l!=0){
        l /= 2;
        count++;
    }

    l = length;

    for(int i = 0; i < count; i++){
        std::string name = "VoxelMipLevel";
        name.push_back('0'+i);
        Renderer::ResManager->CreateTexture3D(
            name,
            ResourceDesc {
                (unsigned int)l, 
                (unsigned int)l,
                ResourceEnum::Format::R32G32B32A32_FLOAT,
                ResourceEnum::Type::Texture3D,
                ResourceEnum::State::Write,
            },
            l,
            1<<ResourceEnum::View::UAView|1<<ResourceEnum::View::SRView,
            1
        );
        l /= 2;
    }

    Renderer::ResManager->CreateTexture3D(
        "VoxelMip",
        ResourceDesc {
            length, 
            length,
            ResourceEnum::Format::R32G32B32A32_FLOAT,
            ResourceEnum::Type::Texture3D,
            ResourceEnum::State::Write,
        },
        length,
        1<<ResourceEnum::View::UAView|1<<ResourceEnum::View::SRView,
        count
    );

    ID3D12Resource* res = Renderer::ResManager->GetResource("VoxelMip");

    int dimLength = length;

    for(int i = 0; i < count; i++){
        Graphics::heapMgr->GetNewSRV(mipCpu[i], mipGpu[i]);
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        ZeroMemory(&uavDesc, sizeof(uavDesc));
	    uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        uavDesc.Texture3D.MipSlice = i;
        uavDesc.Texture3D.FirstWSlice = 0;
        uavDesc.Texture3D.WSize = dimLength;
        Device::GetDevice()->CreateUnorderedAccessView(res, nullptr, &uavDesc, mipCpu[i]);
        dimLength /= 2;
    }
}

void Mipmap::ProcessMipmap(unsigned int length)
{
    unsigned int l = length, count = 0;
    while(l!=0){
        l /= 2;
        count++;
    }
    l = length;

    auto res = Renderer::ResManager->GetResource("VoxelMip");

    // voxelMipLevelx state: unorder access -> copy source -> unorder access
    // voxelMip state: unorder access / shader resource -> copy dest -> unorder access / shader resource

    auto voxelState = Renderer::ResManager->GetResourceState("VoxelMip");
    D3D12_RESOURCE_STATES fromState;
    if(voxelState == ResourceEnum::State::Read)
        fromState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    else
        fromState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

    Context::GetContext()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        res,
        fromState,
        D3D12_RESOURCE_STATE_COPY_DEST
        )
    );

    for(int i = 0; i < count; i++){
        std::string mipName = "VoxelMipLevel";
        mipName.push_back('0'+i);
        auto fromVoxel = Renderer::ResManager->GetResource(mipName);

        Context::GetContext()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            fromVoxel,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_COPY_SOURCE
            )
        );

        Renderer::GContext->GetContext()->CopyTextureRegion(
            &CD3DX12_TEXTURE_COPY_LOCATION(res, i),
            0, 0, 0,
            &CD3DX12_TEXTURE_COPY_LOCATION(fromVoxel, 0),
            nullptr
        );
        l /= 2;

        Context::GetContext()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            fromVoxel,
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS
            )
        );
    }

    Context::GetContext()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        res,
        D3D12_RESOURCE_STATE_COPY_DEST,
        fromState
        )
    );
}