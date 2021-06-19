#include "ResourceFatory.hpp"
#include "Device.hpp"
#include "Context.hpp"
#include "WICTextureLoader12.h"

namespace ResFatory {
    void CreateCubeMapResource(ComPtr<ID3D12Resource>& resource, unsigned int width, unsigned int height, unsigned int mipLevels)
    {
        D3D12_RESOURCE_DESC texDesc;
        ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
    
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Alignment = 0;
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.DepthOrArraySize = 6;
        texDesc.MipLevels = mipLevels;
        texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    
        ThrowIfFailed(Device::GetDevice()->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            nullptr,
            IID_PPV_ARGS(&resource)
            )
        );
    }

    void CreateImageTexture(ComPtr<ID3D12Resource>& resource, ComPtr<ID3D12Resource>& uploadBuffer, std::string fn)
    {
        std::wstring wstringFn = std::wstring(fn.begin(), fn.end());
        const wchar_t* wcharFn = wstringFn.c_str();
        std::unique_ptr<uint8_t[]> wicData;
        D3D12_SUBRESOURCE_DATA textureData;
        // load resource data from disk
        ThrowIfFailed(DirectX::LoadWICTextureFromFile(
            Device::GetDevice(),
            wcharFn,
            &resource,
            wicData, 
            textureData)
        );
        D3D12_RESOURCE_DESC textureDesc = resource->GetDesc();
        // Create the GPU upload buffer.
        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(resource.Get(), 0, 1);	
        ThrowIfFailed(Device::GetDevice()->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&uploadBuffer)
            )
        );
        // upload the texture data to GPU
        UpdateSubresources(Context::GetContext(), resource.Get(), uploadBuffer.Get(), 0, 0, 1, &textureData);
        Context::GetContext()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    }

    void CreateRenderTarget2DResource(ComPtr<ID3D12Resource>& resource, unsigned int width, unsigned int height, DXGI_FORMAT format)
    {
        CD3DX12_RESOURCE_DESC texDesc(
            D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            0,		// alignment
            width, height, 1,
            1,		// mip levels
            format,
            1, 0,	// sample count/quality
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        );

        Device::GetDevice()->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, 0, 0),
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            nullptr, // &clearValue,
            IID_PPV_ARGS(&resource)
        );
    }

    void CreateDepthStencil(ComPtr<ID3D12Resource>& resource, unsigned int width, unsigned int height, DXGI_FORMAT format)
    {
        D3D12_RESOURCE_DESC texDesc;
        ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Alignment = 0;
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.DepthOrArraySize = 1;
        texDesc.MipLevels = 1;
        texDesc.Format = format;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        ThrowIfFailed(Device::GetDevice()->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            nullptr,
            IID_PPV_ARGS(&resource)
            )
        );
    }

    void CreateTexture3D(ComPtr<ID3D12Resource>& resource, unsigned int x, unsigned int y, unsigned int z, DXGI_FORMAT format)
    {
        CD3DX12_RESOURCE_DESC texDesc(
            D3D12_RESOURCE_DIMENSION_TEXTURE3D,
            0,		// alignment
            x, y, z,
            1,		// mip levels
            format,
            1, 0,	// sample count/quality
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
        );

        Device::GetDevice()->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, 0, 0),
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            nullptr, // &clearValue,
            IID_PPV_ARGS(&resource)
        );
    }
};