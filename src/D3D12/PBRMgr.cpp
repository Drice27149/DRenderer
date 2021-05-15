#include "PBRMgr.hpp"

PBRMgr::PBRMgr(ID3D12Device* device, ID3D12GraphicsCommandList*  commandList):
PassMgr(device, commandList)
{}

void PBRMgr::Init()
{
    // TODO: Async
    CompileShaders();
    CreateResources();

    BuildRootSig();
    BuildPSO();
}

void PBRMgr::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
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

    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = true;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
}

void PBRMgr::BuildRootSig()
{
    CD3DX12_ROOT_PARAMETER params[20];
    int top = 0;
    // pass constant, obj constant
    // 0, 1
    for(int i = 0; i < 2; i++) params[top++].InitAsConstantBufferView(i);
    // shadowMap, normal, baseColor, metallicRoughness, emissive
    // 2, 3, 4, 5, 6
    unsigned int usedSrv = 0;
    CD3DX12_DESCRIPTOR_RANGE srvTables[5];
    for(unsigned int i = 0; i < 5; i++){
        srvTables[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, usedSrv++);
        params[top++].InitAsDescriptorTable(1, &(srvTables[i]), D3D12_SHADER_VISIBILITY_PIXEL);
    }
    // offset, entry, light info, cluster info
    // 7, 8, 9, 10
    CD3DX12_DESCRIPTOR_RANGE uavTable0;
	uavTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
	CD3DX12_DESCRIPTOR_RANGE uavTable1;
	uavTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
    params[top++].InitAsDescriptorTable(1, &uavTable0);
    params[top++].InitAsDescriptorTable(1, &uavTable1);
	params[top++].InitAsShaderResourceView(usedSrv++, D3D12_SHADER_VISIBILITY_PIXEL);
    params[top++].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(top, params, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

	ThrowIfFailed(device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&rootSig)
        )
    );
}

void PBRMgr::CompileShaders()
{
	vs = d3dUtil::CompileShader(L"..\\assets\\shaders\\shading\\pbr.hlsl", nullptr, "VS", "vs_5_1");
	ps = d3dUtil::CompileShader(L"..\\assets\\shaders\\shading\\pbr.hlsl", nullptr, "PS", "ps_5_1");
}

void PBRMgr::CreateResources()
{
    for(int i = 0; i < aiTextureType_UNKNOWN + 1; i++) mapping[i] = -1;
    mapping[aiTextureType_NORMALS] = 3;
    mapping[aiTextureType_DIFFUSE] = 4;
    // @TODO: metallic roughness = 5
    // mapping[aiTextureType_xxx] = 5;
    mapping[aiTextureType_EMISSIVE] = 6;
}


void PBRMgr::Pass()
{
    auto objectAddr = constantMgr->GetObjectConstant((unsigned long long)0);
    int idOffset = 0, vsOffset = 0;
    
    for(Object* obj: DEngine::gobjs){
        commandList->SetGraphicsRootConstantBufferView(1, objectAddr);
        // set per object image texture for shading
        for(int i = 0; i < aiTextureType_UNKNOWN+1; i++){
            if(obj->mask & (1<<i)){
                unsigned int slot = mapping[i];
                if(slot != -1){
                    commandList->SetGraphicsRootDescriptorTable(slot, textureMgr->GetGPUHandle(obj->texns[i]));
                }
            }
        }
        // rendering
        for(Mesh& mesh: obj->meshes){
            int idSize = mesh.ids.size();

            if(obj->drawType == DrawType::Normal) 
                commandList->DrawIndexedInstanced(idSize, 1, idOffset, vsOffset, 0);

            idOffset += mesh.ids.size();
            vsOffset += mesh.vs.size();
        }

        objectAddr += d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectUniform));
    }
}

void PBRMgr::PrePass()
{
    commandList->SetPipelineState(pso.Get());
    commandList->SetGraphicsRootSignature(rootSig.Get());

    auto passAddr = constantMgr->GetCameraPassConstant();
    commandList->SetGraphicsRootConstantBufferView(0, passAddr);
    commandList->SetGraphicsRootDescriptorTable(2, shadowMgr->srvGpu);
    commandList->SetGraphicsRootDescriptorTable(7, lightCullMgr->GetOffsetHandle());
    commandList->SetGraphicsRootDescriptorTable(8, lightCullMgr->GetEntryHandle());
    commandList->SetGraphicsRootShaderResourceView(9, lightCullMgr->GetLightTable());
    commandList->SetGraphicsRootConstantBufferView(10, lightCullMgr->GetClusterInfo());

    commandList->IASetVertexBuffers(0, 1, &objMesh->VertexBufferView());
    commandList->IASetIndexBuffer(&objMesh->IndexBufferView());
    commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void PBRMgr::PostPass()
{

}
