#pragma once

#include "Resource.hpp"
#include "Struct.hpp"
using namespace Microsoft::WRL;

class PrefilterIBL {
public:
	void Init();
	void PreComputeFilter();
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetDiffuseMap(){ return diffuseG; };
public:
	std::vector<RootEntry> rootors;
	// @TODO: shader map
	ComPtr<ID3DBlob> vs;
    ComPtr<ID3DBlob> ps;

	ComPtr<ID3D12PipelineState> pso;
    ComPtr<ID3D12RootSignature> rootSig;

	ComPtr<ID3D12Resource> diffuseMap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE diffuseC;
	CD3DX12_GPU_DESCRIPTOR_HANDLE diffuseG;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvC[6];
	CD3DX12_GPU_DESCRIPTOR_HANDLE rtvG[6];

	D3D12_RECT scissorRect;
	D3D12_VIEWPORT screenViewport;

	unsigned int width = 512;
	unsigned int height = 512;
};