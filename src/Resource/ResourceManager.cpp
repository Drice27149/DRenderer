#include "ResourceManager.hpp"
#include "RenderStruct.hpp"
#include "Device.hpp"

void ResourceManager::RegisterResource(std::string name, ComPtr<ID3D12Resource>res)
{
    resources[name] = res;
}

void ResourceManager::RegisterHandle(std::string name,CD3DX12_CPU_DESCRIPTOR_HANDLE handle, ResourceEnum::View view)
{
    if(!views.count(name)){
        views[name] = ResourceView();
    }
    views[name].mask |= 1<<view;
    views[name].cpu[view] = handle;
}

void ResourceManager::RegisterHandle(std::string name, CD3DX12_GPU_DESCRIPTOR_HANDLE handle, ResourceEnum::View view)
{
    if(!views.count(name)){
        views[name] = ResourceView();
    }
    views[name].mask |= 1<<view;
    views[name].gpu[view] = handle;
}

void ResourceManager::RegisterHandle(std::string name, CD3DX12_CPU_DESCRIPTOR_HANDLE chandle, CD3DX12_GPU_DESCRIPTOR_HANDLE ghandle, ResourceEnum::View view)
{
    ResourceManager::RegisterHandle(name, chandle, view);
    ResourceManager::RegisterHandle(name, ghandle, view);
}

ID3D12Resource* ResourceManager::GetResource(std::string name)
{
    return resources[name].Get();
}

CD3DX12_CPU_DESCRIPTOR_HANDLE ResourceManager::GetCPU(std::string name, ResourceEnum::View view)
{
    return views[name].cpu[view];
}

CD3DX12_GPU_DESCRIPTOR_HANDLE ResourceManager::GetGPU(std::string name, ResourceEnum::View view)
{
    return views[name].gpu[view];
}

template<typename T>
void ResourceManager::CommitConstantBuffer(std::string name, T* data)
{
    std::shared_ptr<CommonUpload> newItem = std::make_shared<CommonUpload>(Device::GetDevice(), d3dUtil::CalcConstantBufferByteSize(sizeof(T)));
    newItem->CopyData(data, sizeof(T));
    ResourceManager::RegisterResource(name, newItem->Resource());
}

void ResourceManager::ForwardFrame()
{
    frame++;
    while(bufferItems.size()!=0 && bufferItems.front()->frameID<frame)
        bufferItems.pop();
}
