#pragma once

#include "PassMgr.hpp"
#include "ConstantMgr.hpp"
#include "DMesh.hpp"

struct UploadResource {
    ComPtr<ID3D12Resource> Resource = nullptr;
	ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

class SkyBoxMgr: public PassMgr {
public:
    SkyBoxMgr(ID3D12Device* device, 
        ID3D12GraphicsCommandList*  commandList, 
        CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu, 
        CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu
    );
public:
    void Init() override;
    void BuildPSO() override;
    void BuildRootSig() override;
    void CompileShaders() override;
    void CreateResources() override;

    void Pass() override;
    void PrePass() override;
    void PostPass() override;
private:
    std::unique_ptr<UploadResource> skyTexture;
    std::unique_ptr<DMesh> skyMesh = nullptr;
public:
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu;

    // 临时获取常量的接口
    // @TODO: 放入 DEngine 或者全局 context 中
    std::shared_ptr<ConstantMgr> constantMgr = nullptr;
};