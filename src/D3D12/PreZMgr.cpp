#include "PreZMgr.hpp"

PreZMgr::PreZMgr(ID3D12Device* device, ID3D12GraphicsCommandList*  commandList, int width, int height):
    PassMgr(device, commandList),
    width(width),
    height(height)
{

}
    
void PreZMgr::BuildRootSig()
{
	CD3DX12_ROOT_PARAMETER params[2];

    params[0].InitAsConstantBufferView(0);
    params[1].InitAsConstantBufferView(1);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, params, 0 , nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT/*D3D12_ROOT_SIGNATURE_FLAG_NONE*/);
	// CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, params, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if(errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(device->CreateRootSignature(
		    0,
		    serializedRootSig->GetBufferPointer(),
		    serializedRootSig->GetBufferSize(),
		    IID_PPV_ARGS(&rootSig)
        )
    );
}

void PreZMgr::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = true;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 0;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    
    psoDesc.InputLayout = { inputLayout.data(), (unsigned int)inputLayout.size() };
    psoDesc.pRootSignature = rootSig.Get();

    psoDesc.VS = 
	{ 
		reinterpret_cast<BYTE*>(vs->GetBufferPointer()), 
		vs->GetBufferSize() 
	};
    psoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(ps->GetBufferPointer()), 
		ps->GetBufferSize() 
	};

    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
}

void PreZMgr::CreateResources()
{
    depthMap = std::make_shared<Resource>(device, width, height);

    depthMap->srvCpu = srvCpu;
    depthMap->srvGpu = srvGpu;
    depthMap->xxxCpu = dsvCpu;

    depthMap->BuildDepthMap(DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT);
}

void PreZMgr::CompileShaders()
{
	vs = d3dUtil::CompileShader(L"..\\assets\\shaders\\common\\depth.hlsl", nullptr, "VS", "vs_5_1");
	ps = d3dUtil::CompileShader(L"..\\assets\\shaders\\common\\depth.hlsl", nullptr, "PS", "ps_5_1");
}

void PreZMgr::Init()
{
    // TODO: Async
    CompileShaders();
    CreateResources();

    BuildRootSig();
    BuildPSO();
}

void PreZMgr::Pass()
{
    commandList->OMSetRenderTargets(0, nullptr, false, &depthMap->xxxCpu);
    commandList->RSSetViewports(1, &depthMap->Viewport());
    commandList->RSSetScissorRects(1, &depthMap->ScissorRect());

    commandList->ResourceBarrier(1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(
            depthMap->GetResource(),
            D3D12_RESOURCE_STATE_GENERIC_READ, 
            D3D12_RESOURCE_STATE_DEPTH_WRITE
        )
    );

    commandList->ClearDepthStencilView(depthMap->xxxCpu, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    commandList->SetPipelineState(pso.Get()); 

    auto passAddr = constantMgr->GetCameraPassConstant();
    commandList->SetGraphicsRootConstantBufferView(0, passAddr);

    // DrawObjects(DrawType::Normal);

    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            depthMap->GetResource(),
            D3D12_RESOURCE_STATE_DEPTH_WRITE, 
            D3D12_RESOURCE_STATE_GENERIC_READ
        )
    );
}

void PreZMgr::PrePass()
{
    commandList->OMSetRenderTargets(0, nullptr, false, &depthMap->xxxCpu);
    commandList->RSSetViewports(1, &depthMap->Viewport());
    commandList->RSSetScissorRects(1, &depthMap->ScissorRect());

    commandList->ResourceBarrier(1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(
            depthMap->GetResource(),
            D3D12_RESOURCE_STATE_GENERIC_READ, 
            D3D12_RESOURCE_STATE_DEPTH_WRITE
        )
    );

    commandList->ClearDepthStencilView(depthMap->xxxCpu, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    commandList->SetPipelineState(pso.Get()); 
    commandList->SetGraphicsRootSignature(rootSig.Get());

    auto passAddr = constantMgr->GetCameraPassConstant();
    commandList->SetGraphicsRootConstantBufferView(0, passAddr);
}

void PreZMgr::PostPass()
{
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            depthMap->GetResource(),
            D3D12_RESOURCE_STATE_DEPTH_WRITE, 
            D3D12_RESOURCE_STATE_GENERIC_READ
        )
    );
}