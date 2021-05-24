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
    void BuildBloomPSO(ComPtr<ID3DBlob>& newVs, ComPtr<ID3DBlob>& newPs, ComPtr<ID3D12PipelineState>& newpso);
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetBloomSrvGpu(){ return srvGpu[2]; }
public:
    D3D12_GPU_DESCRIPTOR_HANDLE input;
    std::unique_ptr<Resource> downSampleRT[9] = { nullptr };
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCpu[9];
    CD3DX12_GPU_DESCRIPTOR_HANDLE rtvGpu[9];
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu[9];
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu[9];
private:
    ComPtr<ID3DBlob> xvs = nullptr;
    ComPtr<ID3DBlob> xps = nullptr;
    ComPtr<ID3DBlob> yvs = nullptr;
    ComPtr<ID3DBlob> yps = nullptr;
    ComPtr<ID3D12PipelineState> xpso;
    ComPtr<ID3D12PipelineState> ypso;
};