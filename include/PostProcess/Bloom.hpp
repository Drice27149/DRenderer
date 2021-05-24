#pragma once

#include "PostProcess.hpp"

class Bloom : public PostProcess {
public:
    void Init() override;
    void CreateResources() override;
    void CompileShaders() override;
    void BuildPSO() override;

    void PrePass() override;
    void PostPass() override;
    void BloomPass();
    void BloomLoop(unsigned int index);
public:
    D3D12_GPU_DESCRIPTOR_HANDLE input;
    std::unique_ptr<Resource> downSampleRT[9] = { nullptr };
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCpu[9];
    CD3DX12_GPU_DESCRIPTOR_HANDLE rtvGpu[9];
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu[9];
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu[9];
private:
    ComPtr<ID3DBlob> fvs = nullptr;
    ComPtr<ID3DBlob> fps = nullptr;
    ComPtr<ID3D12PipelineState> fpso;
};