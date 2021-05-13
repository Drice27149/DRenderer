#include "DebugVisMgr.hpp"

void DebugVisMgr::BuildRootSig()
{
   CD3DX12_ROOT_PARAMETER slotRootParameter[5];
    // 根签名
	CD3DX12_DESCRIPTOR_RANGE uavTable0;
	uavTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE uavTable1;
	uavTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

	// Perfomance TIP: Order from most frequent to least frequent.
    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsConstantBufferView(1);
    slotRootParameter[2].InitAsConstantBufferView(2);
	slotRootParameter[3].InitAsDescriptorTable(1, &uavTable0);
	slotRootParameter[4].InitAsDescriptorTable(1, &uavTable1);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

	ThrowIfFailed(device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(rootSig.GetAddressOf())));
}

void DebugVisMgr::BuildPSO()
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
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

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

    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
}

void DebugVisMgr::CreateResources()
{
}

void DebugVisMgr::CompileShaders()
{
	vs = d3dUtil::CompileShader(L"..\\assets\\shaders\\debug\\cluster.hlsl", nullptr, "VS", "vs_5_1");
    gs = d3dUtil::CompileShader(L"..\\assets\\shaders\\debug\\cluster.hlsl", nullptr, "GS", "gs_5_1");
	ps = d3dUtil::CompileShader(L"..\\assets\\shaders\\debug\\cluster.hlsl", nullptr, "PS", "ps_5_1");
}

void DebugVisMgr::Init()
{
    // TODO: Async
    CompileShaders();
    CreateResources();

    BuildRootSig();
    BuildPSO();
}

void DebugVisMgr::Pass()
{
}

void DebugVisMgr::PrePass()
{
    commandList->SetPipelineState(pso.Get());
    commandList->SetGraphicsRootSignature(rootSig.Get());

    auto passAddr = constantMgr->GetCameraPassConstant();
    commandList->SetGraphicsRootConstantBufferView(0, passAddr);
    // mCommandList->SetGraphicsRootConstantBufferView(3, );
    commandList->SetGraphicsRootConstantBufferView(2, constantMgr->clusterInfo->Resource()->GetGPUVirtualAddress());
    // mCommandList->SetGraphicsRootDescriptorTable(3, HeadTableHandle);
    // mCommandList->SetGraphicsRootDescriptorTable(4, NodeTableHandle);
    commandList->SetGraphicsRootDescriptorTable(3, offsetTable);
    commandList->SetGraphicsRootDescriptorTable(4, entryTable);
}

void DebugVisMgr::PostPass()
{

}