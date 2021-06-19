#pragma once

#include "d3dUtil.h"
using Microsoft::WRL::ComPtr;

class HeapMgr {
public:
    HeapMgr(ID3D12Device* device, ID3D12GraphicsCommandList*  commandList, unsigned int SRV, unsigned int DSV, unsigned int RTV);
    void GetNewSRV(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
    void GetNewDSV(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
    void GetNewRTV(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
    void GetNewUAV(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
    ID3D12DescriptorHeap* GetSRVHeap(){ return SRVHeap.Get(); }
    ID3D12DescriptorHeap* GetDSVHeap(){ return DSVHeap.Get(); }
    ID3D12DescriptorHeap* GetRTVHeap(){ return RTVHeap.Get(); }
public:
    ID3D12Device* device; 
    ID3D12GraphicsCommandList*  commandList;
    ComPtr<ID3D12DescriptorHeap> SRVHeap;
    unsigned int SRVCnt;
    ComPtr<ID3D12DescriptorHeap> DSVHeap;
    unsigned int DSVCnt;
    ComPtr<ID3D12DescriptorHeap> RTVHeap;
    unsigned int RTVCnt;
    ComPtr<ID3D12DescriptorHeap> UAVHeap;
    ComPtr<ID3D12DescriptorHeap> UAVGpuHeap;
    unsigned int UAVCnt;
private:
	unsigned int srvSize;
	unsigned int dsvSize;
    unsigned int rtvSize;
    unsigned int uavSize;
};
