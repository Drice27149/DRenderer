#include "Mipmap.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"
#include "Device.hpp"
#include "Graphics.hpp"

CD3DX12_CPU_DESCRIPTOR_HANDLE Mipmap::mipCpu[3];
CD3DX12_GPU_DESCRIPTOR_HANDLE Mipmap::mipGpu[3];

void Mipmap::CreateMipmap(unsigned int length)
{
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
        3u   
    );

    ID3D12Resource* res = Renderer::ResManager->GetResource("VoxelMip");

    int dimLength = length;

    for(int i = 0; i < 3; i++){
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

}