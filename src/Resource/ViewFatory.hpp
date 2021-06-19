#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include "d3dx12.h"
#include "d3dUtil.h"

using namespace Microsoft::WRL;

namespace ViewFatory {
	void AppendCubeSRV(ComPtr<ID3D12Resource>& resource, DXGI_FORMAT format, CD3DX12_CPU_DESCRIPTOR_HANDLE handle);
    void AppendTexture2DSRV(ComPtr<ID3D12Resource>& resource, DXGI_FORMAT format, CD3DX12_CPU_DESCRIPTOR_HANDLE handle);
    void AppendRTV(ComPtr<ID3D12Resource>& resource, DXGI_FORMAT format, CD3DX12_CPU_DESCRIPTOR_HANDLE handle);
    void AppendDSV(ComPtr<ID3D12Resource>& resource, DXGI_FORMAT format, CD3DX12_CPU_DESCRIPTOR_HANDLE handle);
    void AppendUAV(ComPtr<ID3D12Resource>& resource, D3D12_UAV_DIMENSION viewDim, CD3DX12_CPU_DESCRIPTOR_HANDLE handle);
    void AppendUAV(ID3D12Resource* resource, D3D12_UAV_DIMENSION viewDim, CD3DX12_CPU_DESCRIPTOR_HANDLE handle);
};