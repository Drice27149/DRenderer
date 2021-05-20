#include "PostProcess.hpp" 
#include "Graphics.hpp"

void PostProcess::BuildRootSig()
{
    unsigned int paramsCnt = inputs.size();
    CD3DX12_ROOT_PARAMETER params[15];
    CD3DX12_DESCRIPTOR_RANGE srvTable[15];
    for(int i = 0; i < paramsCnt; i++){
        srvTable[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, i);
        params[i].InitAsDescriptorTable(1, &(srvTable[i]), D3D12_SHADER_VISIBILITY_PIXEL);
    }
	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(paramsCnt, params, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if(errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(Graphics::GDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(rootSig.GetAddressOf())));
}

void PostProcess::BuildPSO()
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
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
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
    psoDesc.GS = { nullptr, 0 };
    psoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(ps->GetBufferPointer()), 
		ps->GetBufferSize() 
	};
    ThrowIfFailed(Graphics::GDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
}

void PostProcess::Pass()
{
    Graphics::GCmdList->DrawInstanced(6, 1, 0, 0);
}


