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

struct BufferItem {
    unsigned int frameID;
    CommonUpload buffer;
};

class ResourceManager {
public:
    void RegisterResource(std::string name, ComPtr<ID3D12Resource>res);
    void RegisterHandle(std::string name, CD3DX12_CPU_DESCRIPTOR_HANDLE handle, ResourceEnum::View view);
    void RegisterHandle(std::string name, CD3DX12_GPU_DESCRIPTOR_HANDLE handle, ResourceEnum::View view);
    void RegisterHandle(std::string name, CD3DX12_CPU_DESCRIPTOR_HANDLE chandle, CD3DX12_GPU_DESCRIPTOR_HANDLE ghandle, ResourceEnum::View view);

    void ForwardFrame();

    // commit cpu constant and get gpu constant address
    template<typename T>
    void CommitConstantBuffer(std::string name, T* data);

    ID3D12Resource* GetResource(std::string name);
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPU(std::string name, ResourceEnum::View view);
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPU(std::string name, ResourceEnum::View view);

    // manage constant buffer's destroy and create
    std::queue<std::shared_ptr<BufferItem>> bufferItems;
public:
    std::map<std::string, ComPtr<ID3D12Resource>> resources;
    std::map<std::string, ResourceView> views;
private:
    unsigned frame = 0;
};