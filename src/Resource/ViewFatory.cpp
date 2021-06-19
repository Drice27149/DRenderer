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

    void AppendDSV(ComPtr<ID3D12Resource>& resource, DXGI_FORMAT format, CD3DX12_CPU_DESCRIPTOR_HANDLE handle)
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc; 
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Format = format;
        dsvDesc.Texture2D.MipSlice = 0;
        Device::GetDevice()->CreateDepthStencilView(resource.Get(), &dsvDesc, handle);
    }

    void AppendUAV(ComPtr<ID3D12Resource>& resource, D3D12_UAV_DIMENSION viewDim, CD3DX12_CPU_DESCRIPTOR_HANDLE handle)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        uavDesc.Format = resource->GetDesc().Format;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        uavDesc.Texture3D.MipSlice = 0;
        uavDesc.Texture3D.FirstWSlice = 0;
        uavDesc.Texture3D.WSize = 1;
        Device::GetDevice()->CreateUnorderedAccessView(resource.Get(), nullptr, &uavDesc, handle);
    }
};