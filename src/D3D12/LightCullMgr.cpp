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
    // nullptr 的地方可以放宏
    cs = d3dUtil::CompileShader(L"..\\assets\\shaders\\cluster\\lightCull.hlsl", nullptr, "CS", "vs_5_1");
}

void LightCullMgr::CreateResources()
{
    // CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu;
    // CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu;
    // heapMgr->GetNewSRV(srvCpu, srvGpu);

    // // debug compute shader texture
    // D3D12_RESOURCE_DESC texDesc;
    // ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
    // texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    // texDesc.Alignment = 0;
    // texDesc.Width = 16;
    // texDesc.Height = 8;
    // texDesc.DepthOrArraySize = 1;
    // texDesc.MipLevels = 1;
    // texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    // texDesc.SampleDesc.Count = 1;
    // texDesc.SampleDesc.Quality = 0;
    // texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    // texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    // ThrowIfFailed(md3dDevice->CreateCommittedResource(
    //     &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
    //     D3D12_HEAP_FLAG_NONE,
    //     &texDesc,
    //     D3D12_RESOURCE_STATE_COMMON,
    //     nullptr,
    //     IID_PPV_ARGS(&debugTexture))
    // );
    // debugTexture->SetName(L"debugTexture");

    // D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

    // uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    // uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    // uavDesc.Texture2D.MipSlice = 0;

    // md3dDevice->CreateUnorderedAccessView(debugTexture.Get(), nullptr, &uavDesc, srvCpu);

    // DebugTableHandle = srvGpu;

	// CD3DX12_RESOURCE_DESC uav_counter_resource_desc(D3D12_RESOURCE_DIMENSION_BUFFER, 0, sizeof(unsigned int), 1, 1, 1,
	// 	DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE);
	// CD3DX12_RESOURCE_DESC uav_counter_uav_resource_desc = uav_counter_resource_desc;
	// uav_counter_uav_resource_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	// md3dDevice->CreateCommittedResource(
	// 	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, 0, 0),
	// 	D3D12_HEAP_FLAG_NONE,
	// 	&uav_counter_uav_resource_desc,
	// 	D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
	// 	nullptr,
	// 	IID_PPV_ARGS(&HeadTableCounter)
    // );

    // heapMgr->GetNewSRV(srvCpu, srvGpu);
    // HeadTableHandle = srvGpu;

    // CD3DX12_RESOURCE_DESC HeadDesc(D3D12_RESOURCE_DIMENSION_BUFFER, 0, (ClusterX*ClusterY*ClusterZ) * sizeof(TempOffset), 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE);
	// HeadDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	// md3dDevice->CreateCommittedResource(
	// 	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, 0, 0),
	// 	D3D12_HEAP_FLAG_NONE,
	// 	&HeadDesc,
	// 	D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
	// 	nullptr,
	// 	IID_PPV_ARGS(&HeadTable)
    // );
    // HeadTable->SetName(L"HeadTable");

	// // still head table, uav desc set up
	// D3D12_UNORDERED_ACCESS_VIEW_DESC lll_uav_view_desc;
	// ZeroMemory(&lll_uav_view_desc, sizeof(lll_uav_view_desc));
	// lll_uav_view_desc.Format = DXGI_FORMAT_UNKNOWN; //Needs to be UNKNOWN for structured buffer
	// lll_uav_view_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	// lll_uav_view_desc.Buffer.FirstElement = 0;
	// lll_uav_view_desc.Buffer.NumElements = ClusterX*ClusterY*ClusterZ;
	// lll_uav_view_desc.Buffer.StructureByteStride = sizeof(TempOffset); //2 uint32s in struct
	// lll_uav_view_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE; //Not a raw view
	// lll_uav_view_desc.Buffer.CounterOffsetInBytes = 0; //First element in UAV counter resource

	// md3dDevice->CreateUnorderedAccessView(HeadTable.Get(), HeadTableCounter.Get(), &lll_uav_view_desc, srvCpu);

    // // node table, contain lightID & next pointer
    // // this table need a counter
	// md3dDevice->CreateCommittedResource(
	// 	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, 0, 0),
	// 	D3D12_HEAP_FLAG_NONE,
	// 	&uav_counter_uav_resource_desc,
	// 	D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
	// 	nullptr,
	// 	IID_PPV_ARGS(&NodeTableCounter)
    // );

    // heapMgr->GetNewSRV(srvCpu, srvGpu);
    // NodeTableHandle = srvGpu;

    // // TODO: 扩大 nodetable 容量为 16*8*24*MaxLight
    // CD3DX12_RESOURCE_DESC NodeDesc(D3D12_RESOURCE_DIMENSION_BUFFER, 0, (2048) * sizeof(TempNode), 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE);
	// NodeDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	// md3dDevice->CreateCommittedResource(
	// 	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, 0, 0),
	// 	D3D12_HEAP_FLAG_NONE,
	// 	&NodeDesc,
	// 	D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
	// 	nullptr,
	// 	IID_PPV_ARGS(&NodeTable)
    // );

    // lll_uav_view_desc.Buffer.NumElements = 2048; // MaxLightCount * clusterX * clusterY * clusterZ
	// lll_uav_view_desc.Buffer.StructureByteStride = sizeof(TempNode); //2 uint32s in struct
	// md3dDevice->CreateUnorderedAccessView(NodeTable.Get(), NodeTableCounter.Get(), &lll_uav_view_desc, srvCpu);
    // NodeTable->SetName(L"NodeTable");

    // // light data look up table
    // vector<TempLight> lights;
    // TempLight l0;
    
    // l0.pos = glm::vec3(1999, 12, 22);
    // l0.radiance = 10.0;
    // for(int i = 0; i < 3; i++){
    //     l0.id = i;
    //     lights.push_back(l0);
    // }
    // unsigned int byteSize = sizeof(TempLight) * lights.size();

    // LightTable = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), lights.data(), byteSize, LightUploadBuffer);
    // LightTable->SetName(L"LightTable");

    // // head table clear buffer
    // vector<TempOffset> cleardata;
    // cleardata.resize(ClusterX*ClusterY*ClusterZ);
    // for(TempOffset& u: cleardata) u.offset = 0;
    // byteSize = sizeof(TempOffset) * cleardata.size();
    // headClearBuffer = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), cleardata.data(), byteSize, headUploadBuffer);

    // // node table counter clear buffer
    // unsigned int counter = 1;
    // nodeCounterClearBuffer = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), &counter, sizeof(unsigned int), nodeUploadBuffer);
}


void LightCullMgr::Pass()
{

}

void LightCullMgr::PrePass()
{

}

void LightCullMgr::PostPass()
{

}
