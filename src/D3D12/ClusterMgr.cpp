#include "ClusterMgr.hpp"

ClusterMgr::ClusterMgr(ID3D12Device* device, 
    ID3D12GraphicsCommandList*  commandList,
    unsigned int clusterX,
    unsigned int clusterY,
    unsigned int clusterZ
):
PassMgr(device, commandList),
clusterX(clusterX),
clusterY(clusterY),
clusterZ(clusterZ)
{

}

void ClusterMgr::Init()
{
    CompileShaders();
    CreateResources();

    BuildRootSig();
    BuildPSO();
}

void ClusterMgr::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = { inputLayout.data(), (unsigned int)inputLayout.size() };
    psoDesc.pRootSignature = rootSig.Get();

	D3D12_RASTERIZER_DESC rasterDescFront;
	rasterDescFront.AntialiasedLineEnable = FALSE;
	rasterDescFront.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	rasterDescFront.CullMode = D3D12_CULL_MODE_NONE;
	rasterDescFront.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterDescFront.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterDescFront.DepthClipEnable = TRUE;
	rasterDescFront.FillMode = D3D12_FILL_MODE_SOLID;
	rasterDescFront.ForcedSampleCount = 0;
	rasterDescFront.FrontCounterClockwise = FALSE;
	rasterDescFront.MultisampleEnable = FALSE;
	rasterDescFront.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;

    psoDesc.RasterizerState = rasterDescFront;
	
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_MAX;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8_UNORM;
	psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;

    psoDesc.VS = {
        reinterpret_cast<BYTE*>(vs->GetBufferPointer()), 
		vs->GetBufferSize() 
    };
    psoDesc.GS = {
        reinterpret_cast<BYTE*>(gs->GetBufferPointer()), 
		gs->GetBufferSize() 
    };
    psoDesc.PS = {
        reinterpret_cast<BYTE*>(ps->GetBufferPointer()), 
		ps->GetBufferSize() 
    };

    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
}

void ClusterMgr::BuildRootSig()
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

void ClusterMgr::CompileShaders()
{
    vs = d3dUtil::CompileShader(L"..\\assets\\shaders\\cluster\\cluster.hlsl", nullptr, "VS", "vs_5_1");
    gs = d3dUtil::CompileShader(L"..\\assets\\shaders\\cluster\\cluster.hlsl", nullptr, "GS", "gs_5_1");
    ps = d3dUtil::CompileShader(L"..\\assets\\shaders\\cluster\\cluster.hlsl", nullptr, "PS", "ps_5_1");
}

void ClusterMgr::CreateResources()
{
    // @TODO: create rtv based on light count
    depthMap = std::make_shared<Resource>(device, clusterX, clusterY, srvCpu, srvGpu, rtvCpu);  
    depthMap->BuildRenderTargetArray(3, DXGI_FORMAT_R8G8_UNORM);
    depthMap->GetResource()->SetName(L"Cluster Depth test");
    // fix camera, for debug
    fixCam = std::make_unique<UploadBuffer<PassUniform>>(device, 1, true);
    // feed in data
    PassUniform temp;
    temp.view = glm::transpose(DEngine::GetCamMgr().GetViewTransform());
    temp.proj = glm::transpose(glm::perspective(45.0, 1.0, 1.0, 20.0));
    fixCam->CopyData(0, temp);

    clusterInfo = std::make_unique<UploadBuffer<ClusterInfo>>(device, 1, true);
    ClusterInfo tempInfo;
    tempInfo.clusterX = clusterX;
    tempInfo.clusterY = clusterY;
    tempInfo.clusterZ = clusterZ;
    tempInfo.cNear = 1.0;
    tempInfo.cFar = 20.0;
    clusterInfo->CopyData(0, tempInfo);
}

void ClusterMgr::Pass()
{

}

void ClusterMgr::PrePass()
{
    commandList->OMSetRenderTargets(1, &rtvCpu, true, nullptr);

    commandList->RSSetViewports(1, &depthMap->Viewport());
    commandList->RSSetScissorRects(1, &depthMap->ScissorRect());

    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthMap->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	
    commandList->SetPipelineState(pso.Get());
    commandList->SetGraphicsRootSignature(rootSig.Get());

    auto passAddr = fixCam->Resource()->GetGPUVirtualAddress();
    commandList->SetGraphicsRootConstantBufferView(0, passAddr);
}

void ClusterMgr::PostPass()
{
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthMap->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}
