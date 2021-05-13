#include "Graphics.hpp"
#include "DEngine.hpp"
#include "GraphicAPI.hpp"
#include "stb_image.h"

const int FrameCount = 3;

Graphics::Graphics(HINSTANCE hInstance)
: D3DApp(hInstance) 
{
}

Graphics::~Graphics()
{
}

bool Graphics::Initialize()
{
    if(!D3DApp::Initialize())
		return false;
		
    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    LoadAssets();

    BuildDescriptorHeaps();

    BuildShadersAndInputLayout();

    InitSRV();
    // order can't be exchanged
    InitUAV();

    PrepareComputeShader();

    PrepareClusterVis();

    BuildFrameResources();
    BuildShaderResourceView();
    BuildRootSignature();
    
    BuildBoxGeometry();
    BuildPSO();
    BuildClusterVisPSO();

    InitPassMgrs();

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

	return true;
}

void Graphics::OnResize()
{
	D3DApp::OnResize();

    // The window resized, so update the aspect ratio and recompute the projection matrix.
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void Graphics::Update(const GameTimer& gt)
{
    constantMgr->Update();
}

void Graphics::Draw(const GameTimer& gt)
{
    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(constantMgr->frameResources[constantMgr->curFrame]->CmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    ThrowIfFailed(mCommandList->Reset(constantMgr->frameResources[constantMgr->curFrame]->CmdListAlloc.Get(), mPSO.Get()));

	// ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	// mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    ID3D12DescriptorHeap* descriptorHeaps[] = { heapMgr->GetSRVHeap() };

    mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    DrawShadowMap();

    PreZPass();

    PrepareCluster();

    ExecuteComputeShader();

    mCommandList->SetPipelineState(mPSO.Get());
    mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

    // Clear the back buffer and depth buffer.
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // shadow map
    mCommandList->SetGraphicsRootDescriptorTable(4, shadowMgr->srvGpu);

    auto passAddr = constantMgr->GetCameraPassConstant();
    mCommandList->SetGraphicsRootConstantBufferView(0, passAddr);

    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

    // real render
    DrawObjects(DrawType::Normal);
    
    // place at last
    DrawSkyBox();

    // debugvis
    DrawLines();

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Done recording commands.
	ThrowIfFailed(mCommandList->Close());
 
    // Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	
	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

    constantMgr->frameResources[constantMgr->curFrame]->Fence = ++mCurrentFence;

    mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void Graphics::BuildDescriptorHeaps()
{
    heapMgr = std::make_unique<HeapMgr>(md3dDevice.Get(), mCommandList.Get(), 50, 50, 50);

    // for(TTexture& texture: textures){
    //     auto tex = texture.Resource;

    //     D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    //     srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    //     srvDesc.Format = tex->GetDesc().Format;
    //     srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    //     srvDesc.Texture2D.MostDetailedMip = 0;
    //     srvDesc.Texture2D.MipLevels = tex->GetDesc().MipLevels;
    //     srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    //     md3dDevice->CreateShaderResourceView(tex.Get(), &srvDesc,hDescriptor);

    //     hDescriptor.Offset(mCbvSrvUavDescriptorSize);
    // }
}

void Graphics::BuildRootSignature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  

	// Root parameter can be a table, root descriptor or root constants.
    const int rootParamsCnt = 7;
	CD3DX12_ROOT_PARAMETER slotRootParameter[rootParamsCnt];

    slotRootParameter[0].InitAsConstantBufferView(0);
    slotRootParameter[1].InitAsConstantBufferView(1);

    // ������������
    CD3DX12_DESCRIPTOR_RANGE texTable;
    texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);
    slotRootParameter[2].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
    // // ������������
    // CD3DX12_DESCRIPTOR_RANGE uniformTable;
    // uniformTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    // slotRootParameter[2].InitAsDescriptorTable(1, &uniformTable);

    CD3DX12_DESCRIPTOR_RANGE passTable;
    passTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
    slotRootParameter[3].InitAsDescriptorTable(1, &passTable);

    // shadow map
    CD3DX12_DESCRIPTOR_RANGE texTable1;
    texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
    slotRootParameter[4].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

    // sky box cube map
    CD3DX12_DESCRIPTOR_RANGE texTable2;
    texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
    slotRootParameter[5].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);

    slotRootParameter[6].InitAsConstantBufferView(3);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(rootParamsCnt, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void Graphics::BuildShadersAndInputLayout()
{
    HRESULT hr = S_OK;
    
	mvsByteCode = d3dUtil::CompileShader(L"..\\shaders\\dx12\\color.hlsl", nullptr, "VS", "vs_5_1");
	mpsByteCode = d3dUtil::CompileShader(L"..\\shaders\\dx12\\color.hlsl", nullptr, "PS", "ps_5_1");

	shadowVS = d3dUtil::CompileShader(L"..\\shaders\\dx12\\shadow.hlsl", nullptr, "VS", "vs_5_1");
	shadowPS = d3dUtil::CompileShader(L"..\\shaders\\dx12\\shadow.hlsl", nullptr, "PS", "ps_5_1");

	skyVS = d3dUtil::CompileShader(L"..\\shaders\\dx12\\sky.hlsl", nullptr, "VS", "vs_5_1");
	skyPS = d3dUtil::CompileShader(L"..\\shaders\\dx12\\sky.hlsl", nullptr, "PS", "ps_5_1");

    clusterVS = d3dUtil::CompileShader(L"..\\shaders\\dx12\\cluster.hlsl", nullptr, "VS", "vs_5_1");
    clusterGS = d3dUtil::CompileShader(L"..\\shaders\\dx12\\cluster.hlsl", nullptr, "GS", "gs_5_1");
    clusterPS = d3dUtil::CompileShader(L"..\\shaders\\dx12\\cluster.hlsl", nullptr, "PS", "ps_5_1");

    clusterCS = d3dUtil::CompileShader(L"..\\shaders\\dx12\\cluster\\lightlist.hlsl", nullptr, "CS", "cs_5_1");

    debugCS = d3dUtil::CompileShader(L"..\\shaders\\dx12\\debug\\debug.hlsl", nullptr, "CS", "cs_5_1");

    clusterVisVS = d3dUtil::CompileShader(L"..\\shaders\\dx12\\debug\\clusterVis.hlsl", nullptr, "VS", "vs_5_1");
    clusterVisGS = d3dUtil::CompileShader(L"..\\shaders\\dx12\\debug\\clusterVis.hlsl", nullptr, "GS", "gs_5_1");
    clusterVisPS = d3dUtil::CompileShader(L"..\\shaders\\dx12\\debug\\clusterVis.hlsl", nullptr, "PS", "ps_5_1");

    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, vertex), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, texCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, tangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, bitangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 1, DXGI_FORMAT_R32_UINT, 0, offsetof(Vertex, x), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 2, DXGI_FORMAT_R32_UINT, 0, offsetof(Vertex, y), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 3, DXGI_FORMAT_R32_UINT, 0, offsetof(Vertex, z), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
}

void Graphics::BuildBoxGeometry()
{
    // �ϲ����� obj ���嵽һ��������
    vector<Vertex> vs;
    vector<unsigned int> ids;
    for(Object* obj: DEngine::gobjs){
        for(Mesh mesh: obj->meshes){
            for(Vertex v: mesh.vs) vs.push_back(v);
            for(unsigned int id: mesh.ids) ids.push_back(id);
        }
    }

    objMesh = std::make_unique<DMesh>();
    objMesh->BuildVertexAndIndexBuffer(md3dDevice.Get(), mCommandList.Get(), vs, ids);
}

void Graphics::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    psoDesc.pRootSignature = mRootSignature.Get();
    psoDesc.VS = 
	{ 
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()), 
		mvsByteCode->GetBufferSize() 
	};
    psoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()), 
		mpsByteCode->GetBufferSize() 
	};

    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = true;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = mBackBufferFormat;
    psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    psoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));

    psoDesc.VS = {
        reinterpret_cast<BYTE*>(clusterVS->GetBufferPointer()), 
		clusterVS->GetBufferSize() 
    };
    psoDesc.GS = {
        reinterpret_cast<BYTE*>(clusterGS->GetBufferPointer()), 
		clusterGS->GetBufferSize() 
    };
    psoDesc.PS = {
        reinterpret_cast<BYTE*>(clusterPS->GetBufferPointer()), 
		clusterPS->GetBufferSize() 
    };

    // cluster depth...
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

    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&clusterPSO)));

    // compute shader, cluster depth pipeline state
    D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc = {};
	computeDesc.pRootSignature = CSRootSignature.Get();
	computeDesc.CS =
	{
		reinterpret_cast<BYTE*>(clusterCS->GetBufferPointer()),
		clusterCS->GetBufferSize()
	};
	computeDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(md3dDevice->CreateComputePipelineState(&computeDesc, IID_PPV_ARGS(&computePSO)));

    D3D12_COMPUTE_PIPELINE_STATE_DESC debugDesc = {};
    debugDesc.pRootSignature = CSRootSignature.Get();
	debugDesc.CS =
	{
		reinterpret_cast<BYTE*>(debugCS->GetBufferPointer()),
		debugCS->GetBufferSize()
	};
    debugDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(md3dDevice->CreateComputePipelineState(&debugDesc, IID_PPV_ARGS(&debugPSO)));
}

void Graphics::LoadAssets()
{
    if(DEngine::gobj == nullptr) return ;
    // Ŀǰ, ����ֻ��һ�� mesh
    const Mesh& mesh = DEngine::gobj->meshes[0];
    if (mesh.mask & (1 << aiTextureType_DIFFUSE)) {
        TTexture baseColorTex;
        CreateTextureFromImage(mesh.texns[aiTextureType_DIFFUSE], baseColorTex.Resource, baseColorTex.UploadHeap);
        textures.push_back(baseColorTex);
    }
    if (mesh.mask & (1 << aiTextureType_NORMALS)) {
        TTexture normalTex;
        CreateTextureFromImage(mesh.texns[aiTextureType_NORMALS], normalTex.Resource, normalTex.UploadHeap);
        textures.push_back(normalTex);
    }
}

// TODO: improve load speed, too slow now 
void Graphics::CreateTextureFromImage(string fn, ComPtr<ID3D12Resource>&m_texture, ComPtr<ID3D12Resource>&textureUploadHeap)
{
	int TextureWidth, TextureHeight;
	int TexturePixelSize;

	stbi_uc* fkImage = stbi_load(fn.c_str(), &TextureWidth, &TextureHeight, &TexturePixelSize, 0);

    if(TexturePixelSize == 3){
        fkImage = stbi_load(fn.c_str(), &TextureWidth, &TextureHeight, &TexturePixelSize, 4);
        TexturePixelSize = 4;
    }
    // Create the texture.
    {
        // Describe and create a Texture2D.
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
       
        if(TexturePixelSize == 4) 
            textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        else if(TexturePixelSize == 2)
            textureDesc.Format = DXGI_FORMAT_R8G8_UNORM;
        else
            textureDesc.Format = DXGI_FORMAT_R8_UNORM;

        textureDesc.Width = TextureWidth;
        textureDesc.Height = TextureHeight;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        ThrowIfFailed(md3dDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_texture)));

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

        // Create the GPU upload buffer.
        ThrowIfFailed(md3dDevice->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&textureUploadHeap)));

        // Copy data to the intermediate upload heap and then schedule a copy 
        // from the upload heap to the Texture2D.
		
        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = fkImage;
        textureData.RowPitch = TextureWidth * TexturePixelSize;
        textureData.SlicePitch = textureData.RowPitch * TextureHeight;

        UpdateSubresources(mCommandList.Get(), m_texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
        mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Graphics::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return { 
		pointWrap, pointClamp,
		linearWrap, linearClamp, 
		anisotropicWrap, anisotropicClamp };
}

void Graphics::OnMouseZoom(WPARAM state)
{
    if(DEngine::instance != nullptr){
        if(GET_WHEEL_DELTA_WPARAM(state) > 0)
            DEngine::GetInputMgr().OnZoomIn();
        else 
            DEngine::GetInputMgr().OnZoomOut();
    }
}

void Graphics::OnLMouseDown(WPARAM btnState, int x, int y)
{
    if(DEngine::instance != nullptr)
        DEngine::GetInputMgr().OnLMouseDown();
}

void Graphics::OnLMouseUp(WPARAM btnState, int x, int y)
{
    if(DEngine::instance != nullptr)
        DEngine::GetInputMgr().OnLMouseRelease();
}

void Graphics::OnRMouseDown(WPARAM btnState, int x, int y)
{
    if(DEngine::instance != nullptr)
        DEngine::GetInputMgr().OnRMouseDown();
}

void Graphics::OnRMouseUp(WPARAM btnState, int x, int y)
{
    if(DEngine::instance != nullptr)
        DEngine::GetInputMgr().OnRMouseRelease();
}

void Graphics::OnMouseMove(WPARAM btnState, int x, int y)
{
    if(DEngine::instance != nullptr)
            DEngine::GetInputMgr().Tick((float)x, (float)y);

    if((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

        // Update angles based on input to orbit camera around box.
        mTheta += dx;
        mPhi += dy;

        // Restrict the angle mPhi.
        mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if((btnState & MK_RBUTTON) != 0)
    {
        // Make each pixel correspond to 0.005 unit in the scene.
        float dx = 0.005f*static_cast<float>(x - mLastMousePos.x);
        float dy = 0.005f*static_cast<float>(y - mLastMousePos.y);

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void Graphics::BuildFrameResources()
{
}

void Graphics::BuildShaderResourceView()
{   
}

void Graphics::DrawShadowMap()
{
    shadowMgr->PrePass();

    DrawObjects(DrawType::Normal);

    shadowMgr->PostPass();
}

void Graphics::DrawSkyBox()
{
    skyBoxMgr->PrePass();
    skyBoxMgr->Pass();
    skyBoxMgr->PostPass();
}

void Graphics::InitDescriptorHeaps()
{
}

void Graphics::InitSRV()
{
    // baseColor/normal/specular + pre-Z + cluster structure
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu;
    heapMgr->GetNewSRV(srvCpu, srvGpu);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE rtvGpu;
    heapMgr->GetNewRTV(rtvCpu, rtvGpu);

    clusterDepth = std::make_unique<Resource>(md3dDevice.Get(), ClusterX, ClusterY, srvCpu, srvGpu, rtvCpu);
    clusterDepth->BuildRenderTargetArray(3, DXGI_FORMAT_R8G8_UNORM);
}

void Graphics::PreZPass()
{
    preZMgr->PrePass();

    DrawObjects(DrawType::Normal);

    preZMgr->PostPass();
}

void Graphics::DrawObjects(DrawType drawType)
{   
    mCommandList->IASetVertexBuffers(0, 1, &objMesh->VertexBufferView());
    mCommandList->IASetIndexBuffer(&objMesh->IndexBufferView());

    if(drawType == DrawType::Normal)
        mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    else if(drawType == DrawType::WhiteLines)
        mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    auto objectAddr = constantMgr->GetObjectConstant((unsigned long long)0);
    int idOffset = 0, vsOffset = 0;
    
    for(Object* obj: DEngine::gobjs){
        mCommandList->SetGraphicsRootConstantBufferView(1, objectAddr);

        for(Mesh mesh: obj->meshes){
            int idSize = mesh.ids.size();
            if(obj->drawType == drawType) 
                mCommandList->DrawIndexedInstanced(idSize, 1, idOffset, vsOffset, 0);
            idOffset += mesh.ids.size();
            vsOffset += mesh.vs.size();
        }

        objectAddr += d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectUniform));
    }
}

void Graphics::DrawLines()
{
    mCommandList->SetPipelineState(clusterVisPSO.Get());
    mCommandList->SetGraphicsRootSignature(clusterVisSignature.Get());

    auto passAddr = constantMgr->GetCameraPassConstant();
    mCommandList->SetGraphicsRootConstantBufferView(0, passAddr);
    // mCommandList->SetGraphicsRootConstantBufferView(3, );
    mCommandList->SetGraphicsRootConstantBufferView(2, constantMgr->clusterInfo->Resource()->GetGPUVirtualAddress());
    // mCommandList->SetGraphicsRootDescriptorTable(3, HeadTableHandle);
    // mCommandList->SetGraphicsRootDescriptorTable(4, NodeTableHandle);
    mCommandList->SetGraphicsRootDescriptorTable(3, lightCullMgr->srvGpu[0]);
    mCommandList->SetGraphicsRootDescriptorTable(4, lightCullMgr->srvGpu[1]);

    DrawObjects(DrawType::WhiteLines);
}

void Graphics::PrepareCluster()
{
    if (fixCamCB == nullptr){
        PassUniform temp;
        temp.view = glm::transpose(DEngine::GetCamMgr().GetViewTransform());
        temp.proj = glm::transpose(glm::perspective(45.0, 1.0, 1.0, 20.0));
        fixCamCB = std::make_unique<UploadBuffer<PassUniform>>(md3dDevice.Get(), 1, true);
        fixCamCB->CopyData(0, temp);
    }

    mCommandList->OMSetRenderTargets(1, &clusterDepth->WriteHandle(), true, nullptr);

    mCommandList->RSSetViewports(1, &clusterDepth->Viewport());
    mCommandList->RSSetScissorRects(1, &clusterDepth->ScissorRect());

    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(clusterDepth->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	// mCommandList->ClearRenderTargetView(clusterDepth->WriteHandle(),  Colors::Black, 0, nullptr);
	
    mCommandList->SetPipelineState(clusterPSO.Get());
    mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

    auto passAddr = fixCamCB->Resource()->GetGPUVirtualAddress();
    mCommandList->SetGraphicsRootConstantBufferView(0, passAddr);

    DrawObjects(DrawType::PointLight);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(clusterDepth->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    clusterMgr->PrePass();
    DrawObjects(DrawType::PointLight);
    clusterMgr->PostPass();
}   

void Graphics::InitUAV()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu;
    heapMgr->GetNewSRV(srvCpu, srvGpu);

    // debug compute shader texture
    D3D12_RESOURCE_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = 16;
    texDesc.Height = 8;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    ThrowIfFailed(md3dDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&debugTexture))
    );
    debugTexture->SetName(L"debugTexture");

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;

    md3dDevice->CreateUnorderedAccessView(debugTexture.Get(), nullptr, &uavDesc, srvCpu);

    DebugTableHandle = srvGpu;

	CD3DX12_RESOURCE_DESC uav_counter_resource_desc(D3D12_RESOURCE_DIMENSION_BUFFER, 0, sizeof(unsigned int), 1, 1, 1,
		DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE);
	CD3DX12_RESOURCE_DESC uav_counter_uav_resource_desc = uav_counter_resource_desc;
	uav_counter_uav_resource_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, 0, 0),
		D3D12_HEAP_FLAG_NONE,
		&uav_counter_uav_resource_desc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&HeadTableCounter)
    );

    heapMgr->GetNewSRV(srvCpu, srvGpu);
    HeadTableHandle = srvGpu;

    CD3DX12_RESOURCE_DESC HeadDesc(D3D12_RESOURCE_DIMENSION_BUFFER, 0, (ClusterX*ClusterY*ClusterZ) * sizeof(TempOffset), 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE);
	HeadDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, 0, 0),
		D3D12_HEAP_FLAG_NONE,
		&HeadDesc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&HeadTable)
    );
    HeadTable->SetName(L"HeadTable");

	// still head table, uav desc set up
	D3D12_UNORDERED_ACCESS_VIEW_DESC lll_uav_view_desc;
	ZeroMemory(&lll_uav_view_desc, sizeof(lll_uav_view_desc));
	lll_uav_view_desc.Format = DXGI_FORMAT_UNKNOWN; //Needs to be UNKNOWN for structured buffer
	lll_uav_view_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	lll_uav_view_desc.Buffer.FirstElement = 0;
	lll_uav_view_desc.Buffer.NumElements = ClusterX*ClusterY*ClusterZ;
	lll_uav_view_desc.Buffer.StructureByteStride = sizeof(TempOffset); //2 uint32s in struct
	lll_uav_view_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE; //Not a raw view
	lll_uav_view_desc.Buffer.CounterOffsetInBytes = 0; //First element in UAV counter resource

	md3dDevice->CreateUnorderedAccessView(HeadTable.Get(), HeadTableCounter.Get(), &lll_uav_view_desc, srvCpu);

    // node table, contain lightID & next pointer
    // this table need a counter
	md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, 0, 0),
		D3D12_HEAP_FLAG_NONE,
		&uav_counter_uav_resource_desc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&NodeTableCounter)
    );

    heapMgr->GetNewSRV(srvCpu, srvGpu);
    NodeTableHandle = srvGpu;

    // TODO: 扩大 nodetable 容量为 16*8*24*MaxLight
    CD3DX12_RESOURCE_DESC NodeDesc(D3D12_RESOURCE_DIMENSION_BUFFER, 0, (2048) * sizeof(TempNode), 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE);
	NodeDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, 0, 0),
		D3D12_HEAP_FLAG_NONE,
		&NodeDesc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&NodeTable)
    );

    lll_uav_view_desc.Buffer.NumElements = 2048; // MaxLightCount * clusterX * clusterY * clusterZ
	lll_uav_view_desc.Buffer.StructureByteStride = sizeof(TempNode); //2 uint32s in struct
	md3dDevice->CreateUnorderedAccessView(NodeTable.Get(), NodeTableCounter.Get(), &lll_uav_view_desc, srvCpu);
    NodeTable->SetName(L"NodeTable");

    // light data look up table
    vector<TempLight> lights;
    TempLight l0;
    
    l0.pos = glm::vec3(1999, 12, 22);
    l0.radiance = 10.0;
    for(int i = 0; i < 3; i++){
        l0.id = i;
        lights.push_back(l0);
    }
    unsigned int byteSize = sizeof(TempLight) * lights.size();

    LightTable = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), lights.data(), byteSize, LightUploadBuffer);
    LightTable->SetName(L"LightTable");

    // head table clear buffer
    vector<TempOffset> cleardata;
    cleardata.resize(ClusterX*ClusterY*ClusterZ);
    for(TempOffset& u: cleardata) u.offset = 0;
    byteSize = sizeof(TempOffset) * cleardata.size();
    headClearBuffer = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), cleardata.data(), byteSize, headUploadBuffer);

    // node table counter clear buffer
    unsigned int counter = 1;
    nodeCounterClearBuffer = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), &counter, sizeof(unsigned int), nodeUploadBuffer);
}

void Graphics::PrepareComputeShader()
{
    // compute shader root signature
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
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(6, slotRootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_NONE);

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

	ThrowIfFailed(md3dDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(CSRootSignature.GetAddressOf())));
}

void Graphics::ExecuteComputeShader()
{
    lightCullMgr->PrePass();
    lightCullMgr->Pass();
    lightCullMgr->PostPass();

    // TODO: clear first
    // ??? clear uav will bug
    // ??? no clear will auto clear
    ClearUAVs();

    int lightCount = 3;

    // compute
    mCommandList->SetPipelineState(computePSO.Get());
    mCommandList->SetComputeRootSignature(CSRootSignature.Get());
    mCommandList->SetComputeRootDescriptorTable(0, HeadTableHandle);
    mCommandList->SetComputeRootDescriptorTable(1, NodeTableHandle);
    mCommandList->SetComputeRootDescriptorTable(2, DebugTableHandle);
    mCommandList->SetComputeRootDescriptorTable(3, clusterDepth->readHandle);
    mCommandList->SetComputeRootShaderResourceView(4, LightTable->GetGPUVirtualAddress());
    mCommandList->SetComputeRootConstantBufferView(5, constantMgr->clusterInfo->Resource()->GetGPUVirtualAddress());
    mCommandList->Dispatch(lightCount, 1, 1);

    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(nullptr));

    mCommandList->SetPipelineState(debugPSO.Get());
    mCommandList->SetComputeRootSignature(CSRootSignature.Get());
    mCommandList->SetComputeRootDescriptorTable(0, HeadTableHandle);
    mCommandList->SetComputeRootDescriptorTable(1, NodeTableHandle);
    mCommandList->SetComputeRootDescriptorTable(2, DebugTableHandle);
    mCommandList->SetComputeRootDescriptorTable(3, clusterDepth->readHandle);
    mCommandList->SetComputeRootShaderResourceView(4, LightTable->GetGPUVirtualAddress());
    mCommandList->SetComputeRootConstantBufferView(5, constantMgr->clusterInfo->Resource()->GetGPUVirtualAddress());
    mCommandList->Dispatch(1, 1, 1);

    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(nullptr));
}

void Graphics::ClearUAVs()
{
    // md3dDevice->TransitionResources(2, mCommandList, resources, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(HeadTable.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(NodeTableCounter.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST));
	
    mCommandList->CopyResource(HeadTable.Get(), headClearBuffer.Get());
    mCommandList->CopyResource(NodeTableCounter.Get(), nodeCounterClearBuffer.Get());
	
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(HeadTable.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(NodeTableCounter.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
}

void Graphics::PrepareClusterVis()
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

	ThrowIfFailed(md3dDevice->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(clusterVisSignature.GetAddressOf())));
}

void Graphics::BuildClusterVisPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    psoDesc.pRootSignature = clusterVisSignature.Get();

    psoDesc.VS = 
	{ 
		reinterpret_cast<BYTE*>(clusterVisVS->GetBufferPointer()), 
		clusterVisVS->GetBufferSize() 
	};
    psoDesc.GS = 
    {
        reinterpret_cast<BYTE*>(clusterVisGS->GetBufferPointer()), 
		clusterVisGS->GetBufferSize() 
    };
    psoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(clusterVisPS->GetBufferPointer()), 
		clusterVisPS->GetBufferSize() 
	};

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
    psoDesc.RTVFormats[0] = mBackBufferFormat;
    psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    psoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&clusterVisPSO)));
}

// 目前只能放在最后, 目前和 baseColor/normal/matellic 的 srvcounter 有冲突的地方
// 重构之后会放到比较前面的位置
void Graphics::InitPassMgrs()
{
    // 常量 (uniform) 管理类
    // @TODO: pass count 准确化
    ID3D12Device* device = md3dDevice.Get();
    ID3D12Fence* fence = mFence.Get();
    constantMgr = std::make_shared<ConstantMgr>(device, fence, (unsigned int)FrameCount, (unsigned int)5, (unsigned int)DEngine::gobjs.size());
    
    // Pre-Z 管理类
    preZMgr = std::make_unique<PreZMgr>(md3dDevice.Get(), mCommandList.Get(), 1024, 1024);
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu;
    heapMgr->GetNewSRV(srvCpu, srvGpu);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE dsvGpu;
    heapMgr->GetNewDSV(dsvCpu, dsvGpu);

    preZMgr->srvCpu = srvCpu;
    preZMgr->srvGpu = srvGpu;
    preZMgr->dsvCpu = dsvCpu;
    // 临时
    preZMgr->constantMgr = constantMgr;
    preZMgr->Init();

    // 阴影管理类
    shadowMgr = std::make_unique<ShadowMgr>(md3dDevice.Get(), mCommandList.Get(), 1024, 1024);
    heapMgr->GetNewSRV(srvCpu, srvGpu);
    heapMgr->GetNewDSV(dsvCpu, dsvGpu);

    shadowMgr->srvCpu = srvCpu;
    shadowMgr->srvGpu = srvGpu;
    shadowMgr->dsvCpu = dsvCpu;
    // 临时
    shadowMgr->constantMgr = constantMgr;
    shadowMgr->Init();

    // 天空盒管理类
    heapMgr->GetNewSRV(srvCpu, srvGpu);
    skyBoxMgr = std::make_unique<SkyBoxMgr>(md3dDevice.Get(), mCommandList.Get(), srvCpu, srvGpu);
    skyBoxMgr->constantMgr = constantMgr;
    skyBoxMgr->Init();

    // light culling step 0: generate depth
    heapMgr->GetNewSRV(srvCpu, srvGpu);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE rtvGpu;
    heapMgr->GetNewRTV(rtvCpu, rtvGpu);
    clusterMgr = std::make_unique<ClusterMgr>(md3dDevice.Get(), mCommandList.Get(), 16, 8, 4);
    clusterMgr->srvCpu = srvCpu;
    clusterMgr->srvGpu = srvGpu;
    clusterMgr->rtvCpu = rtvCpu;
    clusterMgr->rtvGpu = rtvGpu;
    clusterMgr->constantMgr = constantMgr;
    clusterMgr->Init();
    // light culling step 1: cull the light

    lightCullMgr = std::make_unique<LightCullMgr>(md3dDevice.Get(), mCommandList.Get(), 16, 8, 4);
    for(int i = 0; i < 2; i++){
        heapMgr->GetNewSRV(srvCpu, srvGpu);
        lightCullMgr->srvCpu[i] = srvCpu;
        lightCullMgr->srvGpu[i] = srvGpu;
    }
    lightCullMgr->clusterDepth = clusterMgr->srvGpu;
    lightCullMgr->Init();
}