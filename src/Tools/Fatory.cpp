#include <vector>
#include "Fatory.hpp"
#include "d3dx12.h"
#include "Global.hpp"
#include "Graphics.hpp"
#include "WICTextureLoader12.h"

namespace PSOFatory {

    const std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, vertex), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, texCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, tangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TANGENT", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, bitangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 1, DXGI_FORMAT_R32_UINT, 0, offsetof(Vertex, x), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 2, DXGI_FORMAT_R32_UINT, 0, offsetof(Vertex, y), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 3, DXGI_FORMAT_R32_UINT, 0, offsetof(Vertex, z), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

	void CreatePostProcessPSO(ComPtr<ID3D12PipelineState>& pso, ComPtr<ID3DBlob> vs, ComPtr<ID3DBlob> gs, ComPtr<ID3DBlob> ps, ComPtr<ID3D12RootSignature> rootSig)
	{
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
        ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
        psoDesc.InputLayout = { inputLayout.data(), (unsigned int)inputLayout.size() };
        psoDesc.pRootSignature = rootSig.Get();

        D3D12_DEPTH_STENCIL_DESC depthDesc = {};
        depthDesc.DepthEnable = false;

        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.RasterizerState.FrontCounterClockwise = true;
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = depthDesc;// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;
        psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        // build extra pso for grid
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.VS =
        {
            reinterpret_cast<BYTE*>(vs->GetBufferPointer()),
            vs->GetBufferSize()
        };
        if(gs != nullptr) {
            psoDesc.GS = 
            {
                reinterpret_cast<BYTE*>(gs->GetBufferPointer()),
                gs->GetBufferSize()
            };
        }
        psoDesc.PS =
        {
            reinterpret_cast<BYTE*>(ps->GetBufferPointer()),
            ps->GetBufferSize()
        };
        ThrowIfFailed(Graphics::GDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
	}
};

namespace ResourceFatory {
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
    
        ThrowIfFailed(Graphics::GDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            nullptr,
            IID_PPV_ARGS(&resource)
            )
        );
    }

    void CreateTexture2DResource(ComPtr<ID3D12Resource>& resource, ComPtr<ID3D12Resource>& uploadBuffer, std::string fn)
    {
        std::wstring wstringFn = std::wstring(fn.begin(), fn.end());
        const wchar_t* wcharFn = wstringFn.c_str();
        std::unique_ptr<uint8_t[]> wicData;
        D3D12_SUBRESOURCE_DATA textureData;
        // load resource data from disk
        ThrowIfFailed(DirectX::LoadWICTextureFromFile(
            Graphics::GDevice,
            wcharFn,
            &resource,
            wicData, 
            textureData)
        );
        D3D12_RESOURCE_DESC textureDesc = resource->GetDesc();
        // Create the GPU upload buffer.
        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(resource.Get(), 0, 1);	
        ThrowIfFailed(Graphics::GDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&uploadBuffer)
            )
        );
        // upload the texture data to GPU
        UpdateSubresources(Graphics::GCmdList, resource.Get(), uploadBuffer.Get(), 0, 0, 1, &textureData);
        Graphics::GCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    }
};

namespace DescriptorFatory {
	void AppendCubeSRV(ComPtr<ID3D12Resource> resource, DXGI_FORMAT format, CD3DX12_CPU_DESCRIPTOR_HANDLE handle)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.Texture2D.MipLevels = 5;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        Graphics::GDevice->CreateShaderResourceView(resource.Get(), &srvDesc, handle);
    }

    void AppendTexture2DSRV(ComPtr<ID3D12Resource> resource, CD3DX12_CPU_DESCRIPTOR_HANDLE handle)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = resource->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = resource->GetDesc().MipLevels;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        Graphics::GDevice->CreateShaderResourceView(resource.Get(), &srvDesc, handle);
    }
};
