#include "AAMgr.hpp"

void AAMgr::Init() 
{
    // TODO: Async
    CompileShaders();
    CreateResources();

    BuildRootSig();
    BuildPSO();
}

void AAMgr::BuildPSO()
{
    D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc = {};
	computeDesc.pRootSignature = rootSig.Get();
	computeDesc.CS =
	{
		reinterpret_cast<BYTE*>(cs->GetBufferPointer()),
		cs->GetBufferSize()
	};
	computeDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(device->CreateComputePipelineState(&computeDesc, IID_PPV_ARGS(&pso)));
}

void AAMgr::BuildRootSig()
{
    CD3DX12_DESCRIPTOR_RANGE uavTable0;
	uavTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
	CD3DX12_DESCRIPTOR_RANGE uavTable1;
	uavTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

	CD3DX12_ROOT_PARAMETER slotRootParameter[2];
	slotRootParameter[0].InitAsDescriptorTable(1, &uavTable0);
	slotRootParameter[1].InitAsDescriptorTable(1, &uavTable1);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if(errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(rootSig.GetAddressOf())));
}

void AAMgr::CompileShaders()
{
    cs = d3dUtil::CompileShader(L"..\\assets\\shaders\\AA\\SSAA.hlsl", nullptr, "CS", "cs_5_1");
}

void AAMgr::CreateResources()
{
    InRTV = std::make_shared<Resource>(device, commandList);
    CD3DX12_GPU_DESCRIPTOR_HANDLE rtvGpu;
    heapMgr->GetNewRTV(rtvCpu, rtvGpu);
    InRTV->srvCpu = rtvCpu;
    InRTV->srvGpu = rtvGpu;
    InRTV->BuildRenderTarget(sWidth, sHeight, DXGI_FORMAT_R8G8B8A8_UNORM, false);

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

    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(InRTV->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
     D3D12_RESOURCE_STATE_RENDER_TARGET));
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthMap->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE,
     D3D12_RESOURCE_STATE_DEPTH_WRITE));
}

void AAMgr::Update(unsigned int width, unsigned int height)
{
    this->width = width;
    this->height = height;
}

void AAMgr::Pass()
{
    commandList->Dispatch(3, 1, 1);
}

void AAMgr::PrePass()
{

}

void AAMgr::PostPass()
{
    // commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(nullptr));
}


