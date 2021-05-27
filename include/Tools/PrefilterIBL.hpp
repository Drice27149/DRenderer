#pragma once

#include "Resource.hpp"
#include "Struct.hpp"
#include "UploadBuffer.h"
using namespace Microsoft::WRL;

class PrefilterIBL {
public:
	void Init();
	void PreComputeFilter();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetPrefilterEnvMap(){ return envG; };
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetEnvBRDFMap(){ return brdfG; };
	D3D12_GPU_VIRTUAL_ADDRESS GetPassConstantAddr(unsigned long long offset);
public:
	std::vector<RootEntry> rootors;
	// @TODO: shader manager, cache
	ComPtr<ID3DBlob> vs;
    ComPtr<ID3DBlob> ps;

	ComPtr<ID3D12PipelineState> pso;
    ComPtr<ID3D12RootSignature> rootSig;

	ComPtr<ID3D12Resource> brdfMap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE brdfC;
	CD3DX12_GPU_DESCRIPTOR_HANDLE brdfG;
	ComPtr<ID3D12Resource> uploadBuffer;

	ComPtr<ID3D12Resource> envMap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE envC;
	CD3DX12_GPU_DESCRIPTOR_HANDLE envG;
	// overflow issue
	CD3DX12_CPU_DESCRIPTOR_HANDLE sRtvC[30];
	CD3DX12_GPU_DESCRIPTOR_HANDLE sRtvG[30];
	// mip levels for specular ibl
	unsigned int mipLevels = 5;

	// pass constants to evaluate pass id and roughness
	std::unique_ptr<UploadBuffer<PassID>> passConstant = nullptr;

	unsigned int width = 512;
	unsigned int height = 512;
	D3D12_RECT scissorRect[5];
	D3D12_VIEWPORT screenViewport[5];

};