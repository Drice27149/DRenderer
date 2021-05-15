#pragma once

#include <map>
#include "d3dUtil.h"
#include "HeapMgr.hpp"
#include "Resource.hpp"

class TextureMgr {
public:
    TextureMgr(ID3D12Device* device, ID3D12GraphicsCommandList*  commandList):
    device(device), commandList(commandList)
    {}
    void Init();
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(std::string name);

public:
    ID3D12Device* device; 
    ID3D12GraphicsCommandList*  commandList;
public:
    std::shared_ptr<HeapMgr> heapMgr;
    std::vector<std::shared_ptr<Resource>> textures;
    std::map<std::string, CD3DX12_GPU_DESCRIPTOR_HANDLE> handles;
};