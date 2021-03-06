#include "AAMgr.hpp"
#include "Graphics.hpp"


void AAMgr::BuildRootSig()
{
    CD3DX12_ROOT_PARAMETER params[2];
    params[0].InitAsConstantBufferView(0);
    CD3DX12_DESCRIPTOR_RANGE srvTable;
    srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    params[1].InitAsDescriptorTable(1, &srvTable, D3D12_SHADER_VISIBILITY_PIXEL);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, params, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

void AAMgr::BuildPSO()
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
    psoDesc.GS = { nullptr, 0 };
    psoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(ps->GetBufferPointer()), 
		ps->GetBufferSize() 
	};
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
}

void AAMgr::CompileShaders()
{
	// vs = d3dUtil::CompileShader(L"..\\assets\\shaders\\debug\\cluster.hlsl", nullptr, "VS", "vs_5_1");
    // gs = d3dUtil::CompileShader(L"..\\assets\\shaders\\debug\\cluster.hlsl", nullptr, "GS", "gs_5_1");
	// ps = d3dUtil::CompileShader(L"..\\assets\\shaders\\debug\\cluster.hlsl", nullptr, "PS", "ps_5_1");
    vs = d3dUtil::CompileShader(L"..\\assets\\shaders\\AA\\SSAA.hlsl", nullptr, "VS", "vs_5_1");
    ps = d3dUtil::CompileShader(L"..\\assets\\shaders\\AA\\SSAA.hlsl", nullptr, "PS", "ps_5_1");
}

void AAMgr::Init()
{
    // TODO: Async
    CompileShaders();
    CreateResources();

    BuildRootSig();
    BuildPSO();
}

void AAMgr::CreateResources()
{
    // Init AA Pass info, tempory for ssaa
    aaPassInfo = std::make_shared<UploadBuffer<AAPassInfo>>(device, 1, true);
    AAPassInfo passInfo;
    passInfo.ssRate = ssRate;
    aaPassInfo->CopyData(0, passInfo);

    InRTV = std::make_shared<Resource>(device, commandList);
    CD3DX12_GPU_DESCRIPTOR_HANDLE rtvGpu;
    heapMgr->GetNewRTV(rtvCpu, rtvGpu);
    InRTV->srvCpu = rtvCpu;
    InRTV->srvGpu = rtvGpu;
    InRTV->BuildRenderTarget(sWidth, sHeight, DXGI_FORMAT_R32G32B32A32_FLOAT, false);
    heapMgr->GetNewSRV(inSrvCpu, inSrvGpu);
    InRTV->AppendTexture2DSRV(DXGI_FORMAT_R32G32B32A32_FLOAT, inSrvCpu);

    // real rtv
    for(int i = 0; i < 3; i++){
        renderTarget[i] = std::make_shared<Resource>(device, commandList);
        heapMgr->GetNewRTV(rtRtvCpu[i], rtRtvGpu[i]);
        heapMgr->GetNewSRV(rtSrvCpu[i], rtSrvGpu[i]);
        // @TODO hdr support
        renderTarget[i]->BuildRenderTarget(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
        renderTarget[i]->AppendTexture2DRTV(DXGI_FORMAT_R32G32B32A32_FLOAT, rtRtvCpu[i]);
        renderTarget[i]->AppendTexture2DSRV(DXGI_FORMAT_R32G32B32A32_FLOAT, rtSrvCpu[i]);
        if(i == 2)
            renderTarget[i]->mResource->SetName(L"NormalRenderTarget");
    }

    depthMap = std::make_shared<Resource>(device, sWidth, sHeight);
    CD3DX12_GPU_DESCRIPTOR_HANDLE dsvGpu;
    heapMgr->GetNewDSV(dsvCpu, dsvGpu);
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu;
    heapMgr->GetNewSRV(srvCpu, srvGpu);
    depthMap->srvCpu = srvCpu;
    depthMap->srvGpu = srvGpu;
    depthMap->xxxCpu = dsvCpu;
    depthMap->BuildDepthMap(DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT);

    // init render target and depth buffer
    frame = 0;
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        renderTarget[frame]->GetResource(), 
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 
        D3D12_RESOURCE_STATE_RENDER_TARGET
        )
    );
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        depthMap->GetResource(), 
        D3D12_RESOURCE_STATE_COPY_SOURCE,
        D3D12_RESOURCE_STATE_DEPTH_WRITE
        )
    );
}

void AAMgr::Update(unsigned int width, unsigned int height)
{
    this->width = width;
    this->height = height;
}

void AAMgr::Pass()
{
    commandList->SetPipelineState(pso.Get());
    commandList->SetGraphicsRootSignature(rootSig.Get());
    commandList->SetGraphicsRootConstantBufferView(0, aaPassInfo->Resource()->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(1, inSrvGpu);

    commandList->DrawInstanced(6, 1, 0, 0);
}

void AAMgr::PrePass()
{
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        InRTV->GetResource(), 
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 
        D3D12_RESOURCE_STATE_RENDER_TARGET
        )
    );
}

void AAMgr::PostPass()
{
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        InRTV->GetResource(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        )
    );
}

void AAMgr::BeginFrame()
{
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        renderTarget[2]->GetResource(), 
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET
        )
    );
}

void AAMgr::StartTAA()
{
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        renderTarget[2]->GetResource(), 
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        )
    );
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        renderTarget[!frame]->GetResource(), 
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 
        D3D12_RESOURCE_STATE_RENDER_TARGET
        )
    );
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        Graphics::pbrMgr->GetVelocityResource(), 
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        )
    );
    frame = (frame + 1)%2;
}

void AAMgr::EndTAA()
{
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        renderTarget[frame]->GetResource(), 
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        )
    );
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        Graphics::pbrMgr->GetVelocityResource(), 
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET
        )
    );
}

