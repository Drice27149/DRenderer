#include "SkyBoxMgr.hpp"
#include "Graphics.hpp"

SkyBoxMgr::SkyBoxMgr(ID3D12Device* device, 
    ID3D12GraphicsCommandList*  commandList):
    PassMgr(device, commandList)
{
}

void SkyBoxMgr::Init()
{
    CompileShaders();
    CreateResources();

    BuildRootSig();
    BuildPSO();
}

void SkyBoxMgr::BuildPSO()
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
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.InputLayout = { inputLayout.data(), (unsigned int)inputLayout.size() };
    psoDesc.pRootSignature = rootSig.Get();

    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

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

void SkyBoxMgr::BuildRootSig()
{
	CD3DX12_ROOT_PARAMETER params[3];

    params[0].InitAsConstantBufferView(0);
    params[1].InitAsConstantBufferView(1);
    // sky texture
    CD3DX12_DESCRIPTOR_RANGE srvTable;
    srvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    params[2].InitAsDescriptorTable(1, &srvTable, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();
    // CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, params, 0 , nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT/*D3D12_ROOT_SIGNATURE_FLAG_NONE*/);
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, params, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

void SkyBoxMgr::CompileShaders()
{
	vs = d3dUtil::CompileShader(L"..\\assets\\shaders\\skybox\\skybox.hlsl", nullptr, "VS", "vs_5_1");
	ps = d3dUtil::CompileShader(L"..\\assets\\shaders\\skybox\\skybox.hlsl", nullptr, "PS", "ps_5_1");
}

void SkyBoxMgr::CreateResources()
{
    // load cube map first
    skyTexture = std::make_unique<UploadResource>(); 
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(
        device,
        commandList,
        L"..\\assets\\models\\cubeMap\\stdcube1.dds", 
        skyTexture->Resource, 
        skyTexture->UploadHeap
        )
    );
    // create shader resource view for ambient env cube map
    auto skyBox = skyTexture->Resource;
    Graphics::heapMgr->GetNewSRV(cubemapSrvCpu, cubemapSrvGpu);
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = skyBox->GetDesc().MipLevels;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	srvDesc.Format = skyBox->GetDesc().Format;
	device->CreateShaderResourceView(skyBox.Get(), &srvDesc, cubemapSrvCpu);
    // load sky box mesh
    SkyBox skybox;
    skyMesh = std::make_unique<DMesh>();
    skyMesh->BuildVertexAndIndexBuffer(device, commandList, skybox.vs, skybox.ids);
}

void SkyBoxMgr::Pass()
{
    // index count, instance count, index offset, vertex offset, 0
    commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

void SkyBoxMgr::PrePass()
{
    
    commandList->SetPipelineState(pso.Get());
    commandList->SetGraphicsRootSignature(rootSig.Get());

    commandList->IASetVertexBuffers(0, 1, &skyMesh->VertexBufferView());
    commandList->IASetIndexBuffer(&skyMesh->IndexBufferView());
    commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    auto passAddr = Graphics::constantMgr->GetCameraPassConstant();
    commandList->SetGraphicsRootConstantBufferView(0, passAddr);
    // for now, no need to do model transform
    commandList->SetGraphicsRootDescriptorTable(2, cubemapSrvGpu);
}

void SkyBoxMgr::PostPass()
{

}