#include <vector>
#include "Fatory.hpp"
#include "d3dx12.h"
#include "Global.hpp"
#include "Graphics.hpp"

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
        psoDesc.GS = 
        {
            reinterpret_cast<BYTE*>(gs->GetBufferPointer()),
            gs->GetBufferSize()
        };
        psoDesc.PS =
        {
            reinterpret_cast<BYTE*>(ps->GetBufferPointer()),
            ps->GetBufferSize()
        };
        ThrowIfFailed(Graphics::GDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
	}
};
