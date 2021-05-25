#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include "d3dx12.h"
#include "d3dUtil.h"
#include "Struct.hpp"
#include "Graphics.hpp"

using namespace Microsoft::WRL;

class RootEntryFatory {
public:
	static RootEntry CBVEntry(D3D12_GPU_VIRTUAL_ADDRESS addr)
	{
		RootEntry res;
		res.type = RootType::CBV;
		res.addr = addr;
		return res;
	}

	static RootEntry SRVEntry(CD3DX12_GPU_DESCRIPTOR_HANDLE handle)
	{
		RootEntry res;
		res.type = RootType::SRV;
		res.handle = handle;
		return res;
	}
};

class RootSigFatory {
public:
	// use dummy cbv to self define something...
	static void CreateRootSig(ComPtr<ID3D12RootSignature>& rootSig, std::vector<RootEntry>& ens, bool needSampler = false)
	{
		std::vector<CD3DX12_ROOT_PARAMETER> params;
		unsigned int usedCBV = 0;
		unsigned int usedSRV = 0;
		for (auto& en : ens) {
			if (en.type == RootType::CBV) {
				CD3DX12_ROOT_PARAMETER param;
				param.InitAsConstantBufferView(usedCBV++);
				params.emplace_back(param);
			}
			else {
				CD3DX12_ROOT_PARAMETER param;
				CD3DX12_DESCRIPTOR_RANGE srvTable;
				srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, usedSRV++);
				param.InitAsDescriptorTable(1, &srvTable, D3D12_SHADER_VISIBILITY_PIXEL);
				params.emplace_back(param);
			}
		}
		// hack: all rootSig need sampler
		auto staticSamplers = GetStaticSamplers();
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(params.size(), params.data(), (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(Graphics::GDevice->CreateRootSignature(
				0,
				serializedRootSig->GetBufferPointer(),
				serializedRootSig->GetBufferSize(),
				IID_PPV_ARGS(&rootSig)
			)
		);
	}
};

namespace PSOFatory {
	void CreatePostProcessPSO(ComPtr<ID3D12PipelineState>& pso, ComPtr<ID3DBlob> vs, ComPtr<ID3DBlob> gs, ComPtr<ID3DBlob> ps, ComPtr<ID3D12RootSignature> rootSig);
};

namespace ResourceFatory {
	void CreateCubeMapResource(ComPtr<ID3D12Resource>& resource, unsigned int width, unsigned int height);
};

namespace DescriptorFatory {
	void AppendCubeSRV(ComPtr<ID3D12Resource> resource, DXGI_FORMAT format, CD3DX12_CPU_DESCRIPTOR_HANDLE handle);
};
