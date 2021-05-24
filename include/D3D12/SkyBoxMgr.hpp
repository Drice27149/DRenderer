#pragma once

#include "PassMgr.hpp"
#include "ConstantMgr.hpp"
#include "DMesh.hpp"
#include "Resource.hpp"

struct UploadResource {
    ComPtr<ID3D12Resource> Resource = nullptr;
	ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

class SkyBoxMgr: public PassMgr {
public:
    SkyBoxMgr(ID3D12Device* device, ID3D12GraphicsCommandList*  commandList);
public:
    void Init() override;
    void BuildPSO() override;
    void BuildRootSig() override;
    void CompileShaders() override;
    void CreateResources() override;

    void Pass() override;
    void PrePass() override;
    void PostPass() override;

    CD3DX12_GPU_DESCRIPTOR_HANDLE GetCubeMapSrv() { return cubemapSrvGpu;  }
private:
    std::unique_ptr<UploadResource> skyTexture;
    std::unique_ptr<DMesh> skyMesh = nullptr;
public:
    CD3DX12_CPU_DESCRIPTOR_HANDLE cubemapSrvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE cubemapSrvGpu;
};