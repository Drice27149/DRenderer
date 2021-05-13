#include "LightCullMgr.hpp"

LightCullMgr::LightCullMgr(ID3D12Device* device, 
    ID3D12GraphicsCommandList*  commandList,
    unsigned int clusterX,
    unsigned int clusterY,
    unsigned int clusterZ):
PassMgr(device, commandList),
clusterX(clusterX),
clusterY(clusterY),
clusterZ(clusterZ)
{
    
}


void LightCullMgr::Init() 
{
    // TODO: Async
    CompileShaders();
    CreateResources();

    BuildRootSig();
    BuildPSO();
}

void LightCullMgr::BuildPSO()
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

void LightCullMgr::BuildRootSig()
{
    CD3DX12_DESCRIPTOR_RANGE uavTable0;
	uavTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE uavTable1;
	uavTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

	CD3DX12_DESCRIPTOR_RANGE uavTable2;
	uavTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);

    CD3DX12_DESCRIPTOR_RANGE depthTable;
    depthTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[6];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &uavTable0);
	slotRootParameter[1].InitAsDescriptorTable(1, &uavTable1);
	slotRootParameter[2].InitAsDescriptorTable(1, &uavTable2);
    slotRootParameter[3].InitAsDescriptorTable(1, &depthTable);
    slotRootParameter[4].InitAsShaderResourceView(1);
    slotRootParameter[5].InitAsConstantBufferView(0);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(6, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
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

void LightCullMgr::CompileShaders()
{
    cs = d3dUtil::CompileShader(L"..\\assets\\shaders\\cluster\\lightCull.hlsl", nullptr, "CS", "cs_5_1");
}

void LightCullMgr::CreateResources()
{
    offsetTable = std::make_shared<Resource>(device, commandList);
    offsetTable->srvCpu = srvCpu[0];
    offsetTable->srvGpu = srvGpu[0];
    offsetTable->BuildUAV(clusterX*clusterY*clusterZ, sizeof(LightOffset), false);

    // tempory max light count
    unsigned int MAXLIGHTS = 32;
    entryTable = std::make_shared<Resource>(device, commandList);
    entryTable->srvCpu = srvCpu[1];
    entryTable->srvGpu = srvGpu[1];
    entryTable->BuildUAV(clusterX*clusterY*clusterZ*MAXLIGHTS, sizeof(LightEntry), true);

    // tempory light data hack, will be added in shared context
    // because light depth is correct, so the result will be correct
    std::vector<LightInfo> infos;
    for(int i = 0; i < 3; i++){
        LightInfo info;
        info.id = i;
        infos.push_back(info);
    }
    lightTable = std::make_shared<Resource>(device, commandList);
    lightTable->BuildStructureBuffer<LightInfo>(infos.size(), sizeof(LightInfo), infos.data());

    // build helper buffer to clear uavs
    std::vector<LightOffset> cleardata;
    cleardata.resize(clusterX*clusterY*clusterZ);
    for(LightOffset& u: cleardata) u.offset = 0;
    unsigned int byteSize = sizeof(LightOffset) * cleardata.size();
    offsetClearBuffer = d3dUtil::CreateDefaultBuffer(device, commandList, cleardata.data(), byteSize, uploadBuffer);

    // node table counter clear buffer
    unsigned int counter = 1;
    entryClearBuffer = d3dUtil::CreateDefaultBuffer(device, commandList, &counter, sizeof(unsigned int), uploadBuffer);

    
    clusterInfo = std::make_unique<UploadBuffer<ClusterInfo>>(device, 1, true);
    ClusterInfo temp;
    temp.clusterX = clusterX;
    temp.clusterY = clusterY;
    temp.clusterZ = clusterZ;
    temp.cNear = 1.0;
    temp.cFar = 20.0;
    clusterInfo->CopyData(0, temp);
}


void LightCullMgr::Pass()
{
    commandList->Dispatch(3, 1, 1);
}

void LightCullMgr::PrePass()
{
    // clear first
    ClearUAVS();

    commandList->SetPipelineState(pso.Get());
    commandList->SetComputeRootSignature(rootSig.Get());

    commandList->SetComputeRootDescriptorTable(0, srvGpu[0]); // offsetTable
    commandList->SetComputeRootDescriptorTable(1, srvGpu[1]); // entryTable
    // commandList->SetComputeRootUnorderedAccessView(0, );   // debug, not used now
    commandList->SetComputeRootDescriptorTable(3, clusterDepth); // cluster depth
    commandList->SetComputeRootShaderResourceView(4, lightTable->GetResource()->GetGPUVirtualAddress()); // light table buffer
    commandList->SetComputeRootConstantBufferView(5, clusterInfo->Resource()->GetGPUVirtualAddress());
}

void LightCullMgr::PostPass()
{
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(nullptr));
}

void LightCullMgr::ClearUAVS()
{
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(offsetTable->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(entryTable->GetCounterResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));
	
    commandList->CopyResource(offsetTable->GetResource(), offsetClearBuffer.Get());
    commandList->CopyResource(entryTable->GetCounterResource(), entryClearBuffer.Get());
	
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(offsetTable->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(entryTable->GetCounterResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
}
