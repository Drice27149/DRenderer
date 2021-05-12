#include "HeapMgr.hpp"

HeapMgr::HeapMgr(ID3D12Device* device, ID3D12GraphicsCommandList*  commandList, unsigned int SRV, unsigned int DSV, unsigned int RTV):
device(device), commandList(commandList)
{
    D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc;
    SRVHeapDesc.NumDescriptors = SRV;
    SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	SRVHeapDesc.NodeMask = 0;
    ThrowIfFailed(device->CreateDescriptorHeap(&SRVHeapDesc, IID_PPV_ARGS(SRVHeap.GetAddressOf())));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = DSV;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(DSVHeap.GetAddressOf())));

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = RTV;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(RTVHeap.GetAddressOf())));

    srvSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	dsvSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	rtvSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    SRVCnt = DSVCnt = RTVCnt = 0;
}

void HeapMgr::GetNewSRV(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
{
    cpuHandle = SRVHeap->GetCPUDescriptorHandleForHeapStart();
    gpuHandle = SRVHeap->GetGPUDescriptorHandleForHeapStart();
    cpuHandle.Offset(SRVCnt*srvSize);
    gpuHandle.Offset(SRVCnt*srvSize);
    SRVCnt++;
}

void HeapMgr::GetNewDSV(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
{
    cpuHandle = DSVHeap->GetCPUDescriptorHandleForHeapStart();
    gpuHandle = DSVHeap->GetGPUDescriptorHandleForHeapStart();
    cpuHandle.Offset(DSVCnt*dsvSize);
    gpuHandle.Offset(DSVCnt*dsvSize);
    DSVCnt++;
}

void HeapMgr::GetNewRTV(CD3DX12_CPU_DESCRIPTOR_HANDLE& cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
{
    cpuHandle = RTVHeap->GetCPUDescriptorHandleForHeapStart();
    gpuHandle = RTVHeap->GetGPUDescriptorHandleForHeapStart();
    cpuHandle.Offset(RTVCnt*rtvSize);
    gpuHandle.Offset(RTVCnt*rtvSize);
    RTVCnt++;
}