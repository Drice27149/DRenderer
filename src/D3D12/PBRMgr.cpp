#include "PBRMgr.hpp"
#include "Graphics.hpp"
#include "Fatory.hpp"

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
    psoDesc.NumRenderTargets = 2;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    psoDesc.RTVFormats[1] = DXGI_FORMAT_R16G16_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
}

void PBRMgr::BuildRootSig()
{
    CD3DX12_ROOT_PARAMETER params[30];
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
    // 11, diffuse env 
    CD3DX12_DESCRIPTOR_RANGE srvTable;
    srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, usedSrv++);
    params[top++].InitAsDescriptorTable(1, &srvTable, D3D12_SHADER_VISIBILITY_PIXEL);
    // 12 specualr env
    CD3DX12_DESCRIPTOR_RANGE srvTable0;
    srvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, usedSrv++);
    params[top++].InitAsDescriptorTable(1, &srvTable0, D3D12_SHADER_VISIBILITY_PIXEL);
    // 13, scene info for shaindg/shadow, 3
    params[top++].InitAsConstantBufferView(3);

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
    mapping[aiTextureType_LIGHTMAP] = 5;
    mapping[aiTextureType_EMISSIVE] = 6;

    Graphics::heapMgr->GetNewRTV(velRtvCpu, velRtvGpu);
    Graphics::heapMgr->GetNewSRV(velSrvCpu, velSrvGpu);

    ResourceFatory::CreateRenderTarget2DResource(velocity, width, height, DXGI_FORMAT_R16G16_FLOAT);
    velocity->SetName(L"velocity buffer");
    DescriptorFatory::AppendRTV(velocity, DXGI_FORMAT_R16G16_FLOAT, velRtvCpu);
    DescriptorFatory::AppendTexture2DSRV(velocity, DXGI_FORMAT_R16G16_FLOAT, velSrvCpu);
}


void PBRMgr::Pass()
{
    auto objectAddr = Graphics::constantMgr->GetObjectConstant((unsigned long long)0);
    int idOffset = 0, vsOffset = 0;
    
    for(Object* obj: DEngine::gobjs){
        commandList->SetGraphicsRootConstantBufferView(1, objectAddr);
        // set per object image texture for shading
        for(int i = 0; i < aiTextureType_UNKNOWN+1; i++){
            if(obj->mask & (1<<i)){
                unsigned int slot = mapping[i];
                if(slot != -1){
                    commandList->SetGraphicsRootDescriptorTable(slot, Graphics::textureMgr->GetGPUHandle(obj->texns[i]));
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

    auto passAddr = Graphics::constantMgr->GetCameraPassConstant();
    commandList->SetGraphicsRootConstantBufferView(0, passAddr);
    commandList->SetGraphicsRootDescriptorTable(2, Graphics::shadowMgr->srvGpu);
    commandList->SetGraphicsRootDescriptorTable(7, Graphics::lightCullMgr->GetOffsetHandle());
    commandList->SetGraphicsRootDescriptorTable(8, Graphics::lightCullMgr->GetEntryHandle());
    commandList->SetGraphicsRootShaderResourceView(9, Graphics::lightCullMgr->GetLightTable());
    commandList->SetGraphicsRootConstantBufferView(10, Graphics::lightCullMgr->GetClusterInfo());
    commandList->SetGraphicsRootDescriptorTable(11, Graphics::prefilterIBL->GetPrefilterEnvMap());//Graphics::skyBoxMgr->GetCubeMapSrv());
    commandList->SetGraphicsRootDescriptorTable(12, Graphics::prefilterIBL->GetEnvBRDFMap());
    commandList->SetGraphicsRootConstantBufferView(13, Graphics::constantMgr->GetSceneInfoConstant());

    commandList->IASetVertexBuffers(0, 1, &objMesh->VertexBufferView());
    commandList->IASetIndexBuffer(&objMesh->IndexBufferView());
    commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void PBRMgr::PostPass()
{

}
