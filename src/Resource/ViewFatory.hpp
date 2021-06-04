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
};