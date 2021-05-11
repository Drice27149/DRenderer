#pragma once

#include "PassMgr.hpp"
#include "Resource.hpp"
#include "ConstantMgr.hpp"

class PreZMgr: public PassMgr {
public:
    PreZMgr(ID3D12Device* device, ID3D12GraphicsCommandList*  commandList, int width, int height);
    void Init() override;
    void BuildPSO() override;
    void BuildRootSig() override;
    void CompileShaders() override;
    void CreateResources() override;

    void Pass() override;
    void PrePass() override;
    void PostPass() override;
public:
    int width;
    int height;
    std::shared_ptr<Resource> depthMap;
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu;
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvCpu;

public:
    // 临时获取常量的接口
    // @TODO: 放入 DEngine 或者全局 context 中
    std::shared_ptr<ConstantMgr> constantMgr = nullptr;
};