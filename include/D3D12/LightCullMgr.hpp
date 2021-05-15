#pragma once

#include "PassMgr.hpp"
#include "Resource.hpp"
#include "Struct.hpp"
#include "UploadBuffer.h"
#include "ConstantMgr.hpp"

class LightCullMgr: public PassMgr {
public:
    LightCullMgr(ID3D12Device* device, 
        ID3D12GraphicsCommandList*  commandList,
        unsigned int clusterX,
        unsigned int clusterY,
        unsigned int clusterZ
    );
    void Init() override;
    void BuildPSO() override;
    void BuildRootSig() override;
    void CompileShaders() override;
    void CreateResources() override;

    void Pass() override;
    void PrePass() override;
    void PostPass() override;

    CD3DX12_GPU_DESCRIPTOR_HANDLE GetOffsetHandle(){ return srvGpu[0]; } 
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetEntryHandle(){ return srvGpu[1]; } 
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetLightHnadle(){ return srvGpu[2]; } 
    D3D12_GPU_VIRTUAL_ADDRESS GetLightTable(){ return lightTable->GetResource()->GetGPUVirtualAddress(); }
    D3D12_GPU_VIRTUAL_ADDRESS GetClusterInfo(){ return constantMgr->clusterInfo->Resource()->GetGPUVirtualAddress(); }  

    void ClearUAVS();
public:
    // compute shader
    ComPtr<ID3DBlob> cs = nullptr;

    std::shared_ptr<Resource> offsetTable;
    std::shared_ptr<Resource> entryTable;
    std::shared_ptr<Resource> lightTable;

    // helper buffers to clear uavs
    ComPtr<ID3D12Resource> offsetClearBuffer; // clear head table
    ComPtr<ID3D12Resource> entryClearBuffer; // clear node count
    ComPtr<ID3D12Resource> uploadBuffer;

    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu[3];
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu[3];
    CD3DX12_CPU_DESCRIPTOR_HANDLE xxxCpu[3];
    
    CD3DX12_GPU_DESCRIPTOR_HANDLE clusterDepth;
public:
    unsigned int clusterX;
    unsigned int clusterY;
    unsigned int clusterZ;
    // tempory hack
    std::shared_ptr<ConstantMgr> constantMgr;
};