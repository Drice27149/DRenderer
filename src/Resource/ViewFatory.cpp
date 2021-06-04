#include "ViewFatory.hpp"
#include "Device.hpp"

namespace ViewFatory {
	void AppendCubeSRV(ComPtr<ID3D12Resource>& resource, DXGI_FORMAT format, CD3DX12_CPU_DESCRIPTOR_HANDLE handle)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.Texture2D.MipLevels = 5;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        Device::GetDevice()->CreateShaderResourceView(resource.Get(), &srvDesc, handle);
    }

    void AppendTexture2DSRV(ComPtr<ID3D12Resource>& resource, DXGI_FORMAT format, CD3DX12_CPU_DESCRIPTOR_HANDLE handle)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = resource->GetDesc().MipLevels;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        Device::GetDevice()->CreateShaderResourceView(resource.Get(), &srvDesc, handle);
    }

    void AppendRTV(ComPtr<ID3D12Resource>& resource, DXGI_FORMAT format, CD3DX12_CPU_DESCRIPTOR_HANDLE handle)
    {
        D3D12_RENDER_TARGET_VIEW_DESC RTVDesc;
        ZeroMemory(&RTVDesc, sizeof(RTVDesc));
        RTVDesc.Format = resource->GetDesc().Format;
        RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        
        Device::GetDevice()->CreateRenderTargetView(resource.Get(), &RTVDesc, handle);
    }
};