#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount)
{
    ThrowIfFailed(device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

    // FrameCB = std::make_unique<UploadBuffer<FrameConstants>>(device, 1, true);
    // PassCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
    // MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, true);
    // ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
    PassCB = std::make_unique<UploadBuffer<PassUniform>>(device, passCount, true);
    ObjectCB = std::make_unique<UploadBuffer<ObjectUniform>>(device, objectCount, true);
}

FrameResource::~FrameResource()
{
    
}