#pragma once

#include <map>
#include <iostream>
#include <queue>
#include "d3d12.h"
#include "d3dUtil.h"
#include "RenderEnum.hpp"
#include "UploadBuffer.h"

using namespace Microsoft::WRL;

struct ResourceView;
struct ResourceDesc;

struct BufferItem {
    unsigned int frameID;
    std::shared_ptr<CommonUpload> buffer = nullptr;
};

class ResourceManager {
public:
    void CreateRenderTarget(std::string name, ResourceDesc desc, unsigned int usage);
    void CreateImageTexture(std::string name, unsigned int usage);
    void CreateDepthStencil(std::string name, ResourceDesc desc, unsigned int usage);
    // internal function 
    void CreateViews(ComPtr<ID3D12Resource>&, std::string name, unsigned int usage);

    void RegisterResource(std::string name, ComPtr<ID3D12Resource>& res, ResourceEnum::State state = ResourceEnum::State::Read);
    void RegisterHandle(std::string name, CD3DX12_CPU_DESCRIPTOR_HANDLE handle, ResourceEnum::View view);
    void RegisterHandle(std::string name, CD3DX12_GPU_DESCRIPTOR_HANDLE handle, ResourceEnum::View view);
    void RegisterHandle(std::string name, CD3DX12_CPU_DESCRIPTOR_HANDLE chandle, CD3DX12_GPU_DESCRIPTOR_HANDLE ghandle, ResourceEnum::View view);

    ResourceEnum::State GetResourceState(std::string name);
    void ResourceBarrier(std::string name, ResourceEnum::State dest);

    void LoadObjectTextures();

    void ForwardFrame();
    // delete outdated resource
    void ResourceManager::ForwardResource();

    // commit cpu constant to gpu
    template<typename T>
    void CommitConstantBuffer(std::string name, T* data);

    ID3D12Resource* GetResource(std::string name);
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPU(std::string name, ResourceEnum::View view);
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPU(std::string name, ResourceEnum::View view);

    // manage constant buffer's destroy and create
    std::queue<BufferItem> bufferItems;
public:
    std::map<std::string, ComPtr<ID3D12Resource>> resources;
    std::map<std::string, ResourceView> views;
    std::map<std::string, ResourceEnum::State> stateTrack;
private:
    // just keep ref
    std::vector<ComPtr<ID3D12Resource>> resPools;
    unsigned frame = 0;
};

template<typename T>
void ResourceManager::CommitConstantBuffer(std::string name, T* data)
{
    BufferItem newItem;
    newItem.frameID = frame;
    newItem.buffer = std::make_shared<CommonUpload>(Device::GetDevice(), d3dUtil::CalcConstantBufferByteSize(sizeof(T)));
    (newItem.buffer)->CopyData(data, sizeof(T));
    ResourceManager::RegisterResource(name, (newItem.buffer)->Resource());
    // keep ref
    bufferItems.emplace(newItem);
}