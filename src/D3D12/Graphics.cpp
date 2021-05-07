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

    LoadCubeMap();

    BuildDescriptorHeaps();

    BuildShadersAndInputLayout();

    InitSRV();
    // order can't be exchanged
    InitUAV();
    PrepareComputeShader();

    BuildFrameResources();
    BuildShaderResourceView();
    BuildRootSignature();
    
    BuildBoxGeometry();
    BuildPSO();

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

void Graphics::UpdateObjUniform()
{
    auto objCB = mFrameResources[CurrentFrame]->ObjectCB.get();
    auto objAddr = mFrameResources[CurrentFrame]->ObjectCB->Resource()->GetGPUVirtualAddress();
    unsigned int objByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectUniform));

    int index = 0;
    for(Object* obj: DEngine::gobjs){
        ObjectUniform temp;
        temp.model = glm::transpose(obj->model);
        temp.id = obj->id;
        objCB->CopyData(index, temp);
        index++;

        objAddr += objByteSize;
    }
}

void Graphics::UpdatePassUniform()
{
    glm::mat4 tempLight = glm::lookAt(vec3(-8.0, 8.0, 0.0), vec3(0.0,0.0,0.0), vec3(0.0,1.0,0.0));
    auto passCB = mFrameResources[CurrentFrame]->PassCB.get();
    PassUniform temp;
    temp.view = glm::transpose(tempLight); // glm::transpose(DEngine::GetCamMgr().GetViewTransform());
    temp.proj = glm::transpose(DEngine::GetCamMgr().GetProjectionTransform());
    passCB->CopyData(0, temp);
    temp.view = glm::transpose(DEngine::GetCamMgr().GetViewTransform());
    temp.proj = glm::transpose(DEngine::GetCamMgr().GetProjectionTransform());
    temp.SMView = glm::transpose(tempLight);
    temp.SMProj = glm::transpose(DEngine::GetCamMgr().GetProjectionTransform());
    passCB->CopyData(1, temp);
}

void Graphics::Update(const GameTimer& gt)
{
    CurrentFrame = (CurrentFrame + 1)%FrameCount;
    auto mCurFrameResource = mFrameResources[CurrentFrame].get();

    if(mCurFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurFrameResource->Fence){
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(mCurFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    UpdatePassUniform();
    UpdateObjUniform();
}

void Graphics::Draw(const GameTimer& gt)
{
    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(mFrameResources[CurrentFrame]->CmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    ThrowIfFailed(mCommandList->Reset(mFrameResources[CurrentFrame]->CmdListAlloc.Get(), mPSO.Get()));

	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

    DrawShadowMap();

    PreZPass();

    PrepareCluster();

    ExecuteComputeShader();

    mCommandList->SetPipelineState(mPSO.Get());

	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

    // Clear the back buffer and depth buffer.
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // ������ͼ
    mCommandList->SetGraphicsRootDescriptorTable(2, mCbvHeap->GetGPUDescriptorHandleForHeapStart());
    // �����Ѿ�û��
    // mCommandList->SetGraphicsRootConstantBufferView(0, mObjectCB->Resource()->GetGPUVirtualAddress());
    // shadow map ����
    mCommandList->SetGraphicsRootDescriptorTable(4, GPUSMHandle);

    auto passAddr = mFrameResources[CurrentFrame]->PassCB->Resource()->GetGPUVirtualAddress() + d3dUtil::CalcConstantBufferByteSize(sizeof(PassUniform));
    mCommandList->SetGraphicsRootConstantBufferView(0, passAddr);

    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

    DrawObjects(DrawType::Normal);

    DrawLines();

    // place at last
    DrawSkyBox();

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

    mFrameResources[CurrentFrame]->Fence = ++mCurrentFence;

    mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void Graphics::BuildDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = 20;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
        IID_PPV_ARGS(&mCbvHeap)));

    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
        IID_PPV_ARGS(&SrvHeap)));

    // for shadow map
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 5;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
        &dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = 5;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(RTVHeap.GetAddressOf())));

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
    SMHandle = hDescriptor;
    GPUSMHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
    // GPUSMHandle.Offset(2*mCbvSrvUavDescriptorSize);
    hDescriptor.Offset(mCbvSrvUavDescriptorSize);

    auto SkyTex = CubeTex.Resource;
    assert(SkyTex);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = SkyTex->GetDesc().MipLevels;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	srvDesc.Format = SkyTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(SkyTex.Get(), &srvDesc, hDescriptor);

    hDescriptor.Offset(mCbvSrvUavDescriptorSize);

    for(TTexture& texture: textures){
        auto tex = texture.Resource;

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = tex->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = tex->GetDesc().MipLevels;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        md3dDevice->CreateShaderResourceView(tex.Get(), &srvDesc,hDescriptor);

        hDescriptor.Offset(mCbvSrvUavDescriptorSize);
    }
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

    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, vertex), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, texCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, tangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, bitangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
}

void Graphics::BuildBoxGeometry()
{
    SkyBox skybox;

    skyMesh = std::make_unique<DMesh>();
    skyMesh->BuildVertexAndIndexBuffer(md3dDevice.Get(), mCommandList.Get(), skybox.vs, skybox.ids);

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

    // shadow map's pipeline state
    psoDesc.VS = 
    {
		reinterpret_cast<BYTE*>(shadowVS->GetBufferPointer()), 
		shadowVS->GetBufferSize() 
    };
    psoDesc.PS = 
    {
		reinterpret_cast<BYTE*>(shadowPS->GetBufferPointer()), 
		shadowPS->GetBufferSize() 
    };
    psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    psoDesc.NumRenderTargets = 0;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&SMPSO)));

    // skybox texture
    psoDesc.VS = 
    {
		reinterpret_cast<BYTE*>(skyVS->GetBufferPointer()), 
		skyVS->GetBufferSize() 
    };
    psoDesc.PS = 
    {
		reinterpret_cast<BYTE*>(skyPS->GetBufferPointer()), 
		skyPS->GetBufferSize() 
    };
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = mBackBufferFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&SkyPSO)));

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
	rasterDescFront.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
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
    for(int i = 0; i < FrameCount; i++){
        auto newFrameResource = std::make_unique<FrameResource>(md3dDevice.Get(), (unsigned int)PassCount, (unsigned int)(DEngine::gobjs.size()));
        mFrameResources.push_back(std::move(newFrameResource));
    }
}

void Graphics::BuildShaderResourceView()
{   
    shadowMap = std::make_unique<ShadowMap>(md3dDevice.Get(), 2048, 2048);
    shadowMap->BuildDescriptors(SMHandle, GPUSMHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE(mDsvHeap->GetCPUDescriptorHandleForHeapStart()));
}

void Graphics::DrawShadowMap()
{
    mCommandList->RSSetViewports(1, &shadowMap->Viewport());
    mCommandList->RSSetScissorRects(1, &shadowMap->ScissorRect());

    mCommandList->OMSetRenderTargets(0, nullptr, false, &shadowMap->Dsv());

    mCommandList->RSSetViewports(1, &shadowMap->Viewport());
    mCommandList->RSSetScissorRects(1, &shadowMap->ScissorRect());

    // Change to DEPTH_WRITE.
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowMap->Resource(),
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

    // Clear the back buffer and depth buffer.
    mCommandList->ClearDepthStencilView(shadowMap->Dsv(), 
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // // Set null render target because we are only going to draw to
    // // depth buffer.  Setting a null render target will disable color writes.
    // // Note the active PSO also must specify a render target count of 0.

    mCommandList->SetPipelineState(SMPSO.Get());

    auto passAddr = mFrameResources[CurrentFrame]->PassCB->Resource()->GetGPUVirtualAddress();

    mCommandList->SetGraphicsRootConstantBufferView(0, passAddr);

    DrawObjects(DrawType::Normal);

    // Change back to GENERIC_READ so we can read the texture in a shader.
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(shadowMap->Resource(),D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void Graphics::LoadCubeMap()
{
    auto fn = L"..\\assets\\cubeMap\\grasscube1024.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(), mCommandList.Get(), L"..\\assets\\cubeMap\\grasscube1024.dds", CubeTex.Resource, CubeTex.UploadHeap));
}

void Graphics::DrawSkyBox()
{
    mCommandList->SetPipelineState(SkyPSO.Get());
    
    mCommandList->IASetVertexBuffers(0, 1, &skyMesh->VertexBufferView());
    mCommandList->IASetIndexBuffer(&skyMesh->IndexBufferView());
    mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 
    auto skyHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
    skyHandle.Offset(mCbvSrvUavDescriptorSize);
    mCommandList->SetGraphicsRootDescriptorTable(5, skyHandle);
    // index count, instance count, index offset, vertex offset, 0
    mCommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

void Graphics::InitDescriptorHeaps()
{
    // �����кܶ�, ��ʱ���� 32 ��������
    // 1. Pre-Z
    D3D12_DESCRIPTOR_HEAP_DESC desc;
    desc.NumDescriptors = 32;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&desc,
        IID_PPV_ARGS(&SrvHeap)));
    SrvCounter = 0;
}

void Graphics::InitSRV()
{
    // baseColor/normal/specular + pre-Z + cluster structure

    auto CpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvHeap->GetCPUDescriptorHandleForHeapStart());
    CpuHandle.Offset(TextureCount * mCbvSrvUavDescriptorSize);
    auto GpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvHeap->GetGPUDescriptorHandleForHeapStart());
    GpuHandle.Offset(TextureCount * mCbvSrvUavDescriptorSize);

    PreZMap = std::make_unique<Resource>(md3dDevice.Get(), DXGI_FORMAT_D24_UNORM_S8_UINT, mClientWidth, mClientHeight); 
   
    // ��ʱ dsv handle ����Ч��
    auto DsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mDsvHeap->GetCPUDescriptorHandleForHeapStart());
    DsvHandle.Offset(mDsvDescriptorSize);
    PreZMap->BuildDescriptors(CpuHandle, GpuHandle, DsvHandle);

    CpuHandle.Offset(mCbvSrvUavDescriptorSize);
    GpuHandle.Offset(mCbvSrvUavDescriptorSize);

    clusterDepth = std::make_unique<Resource>(md3dDevice.Get(), 
        ClusterX, ClusterY, 
        CpuHandle, GpuHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE(RTVHeap->GetCPUDescriptorHandleForHeapStart())
    );
    clusterDepth->BuildRenderTargetArray(3, DXGI_FORMAT_R8G8_UNORM);

    SrvCounter = TextureCount + 1;
}

void Graphics::PreZPass()
{
    // Set null render target because we are only going to draw to
    // depth buffer.  Setting a null render target will disable color writes.
    // Note the active PSO also must specify a render target count of 0.
    mCommandList->OMSetRenderTargets(0, nullptr, false, &PreZMap->Dsv());

    mCommandList->RSSetViewports(1, &PreZMap->Viewport());
    mCommandList->RSSetScissorRects(1, &PreZMap->ScissorRect());

    // Change to DEPTH_WRITE.
    mCommandList->ResourceBarrier(1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(
            PreZMap->GetResource(),
            D3D12_RESOURCE_STATE_GENERIC_READ, 
            D3D12_RESOURCE_STATE_DEPTH_WRITE
        )
    );

    // Clear the back buffer and depth buffer.
    mCommandList->ClearDepthStencilView(PreZMap->Dsv(), 
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // ��ʱ, ʹ�� shadow pass �� pipeline state object ������Ч��
    mCommandList->SetPipelineState(SMPSO.Get()); 

    auto passAddr = mFrameResources[CurrentFrame]->PassCB->Resource()->GetGPUVirtualAddress() + d3dUtil::CalcConstantBufferByteSize(sizeof(PassUniform));

    mCommandList->SetGraphicsRootConstantBufferView(0, passAddr);

    DrawObjects(DrawType::Normal);

    // Change back to GENERIC_READ so we can read the texture in a shader.
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            PreZMap->GetResource(),
            D3D12_RESOURCE_STATE_DEPTH_WRITE, 
            D3D12_RESOURCE_STATE_GENERIC_READ
        )
    );
}

void Graphics::DrawObjects(DrawType drawType)
{   
    mCommandList->IASetVertexBuffers(0, 1, &objMesh->VertexBufferView());
    mCommandList->IASetIndexBuffer(&objMesh->IndexBufferView());

    if(drawType == DrawType::Normal)
        mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    else if(drawType == DrawType::WhiteLines)
        mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    auto objectAddr = mFrameResources[CurrentFrame]->ObjectCB->Resource()->GetGPUVirtualAddress();    
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
    DrawObjects(DrawType::WhiteLines);
}

void Graphics::PrepareCluster()
{
    mCommandList->OMSetRenderTargets(1, &clusterDepth->WriteHandle(), true, nullptr);

    mCommandList->RSSetViewports(1, &clusterDepth->Viewport());
    mCommandList->RSSetScissorRects(1, &clusterDepth->ScissorRect());

    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(clusterDepth->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
	mCommandList->ClearRenderTargetView(clusterDepth->WriteHandle(),  Colors::Black, 0, nullptr);
	
    mCommandList->SetPipelineState(clusterPSO.Get());

    DrawObjects(DrawType::PointLight);

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(clusterDepth->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}   

void Graphics::InitUAV()
{
    auto CpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvHeap->GetCPUDescriptorHandleForHeapStart());
    auto GpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvHeap->GetGPUDescriptorHandleForHeapStart());
    CpuHandle.Offset(SrvCounter * mCbvSrvUavDescriptorSize);
    GpuHandle.Offset(SrvCounter * mCbvSrvUavDescriptorSize);
    SrvCounter++;

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

    md3dDevice->CreateUnorderedAccessView(debugTexture.Get(), nullptr, &uavDesc, CpuHandle);

    DebugTableHandle = GpuHandle;

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

    // head table
    // fixed size (clusterX * clusterY), no need for counter
    CpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvHeap->GetCPUDescriptorHandleForHeapStart());
    GpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvHeap->GetGPUDescriptorHandleForHeapStart());
    CpuHandle.Offset(SrvCounter*mCbvSrvUavDescriptorSize);
    GpuHandle.Offset(SrvCounter*mCbvSrvUavDescriptorSize);
    SrvCounter++;

    HeadTableHandle = GpuHandle;

    CD3DX12_RESOURCE_DESC HeadDesc(D3D12_RESOURCE_DIMENSION_BUFFER, 0, (ClusterX*ClusterY) * sizeof(TempOffset), 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE);
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
	lll_uav_view_desc.Buffer.NumElements = ClusterX*ClusterY;
	lll_uav_view_desc.Buffer.StructureByteStride = sizeof(TempOffset); //2 uint32s in struct
	lll_uav_view_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE; //Not a raw view
	lll_uav_view_desc.Buffer.CounterOffsetInBytes = 0; //First element in UAV counter resource

	md3dDevice->CreateUnorderedAccessView(HeadTable.Get(), HeadTableCounter.Get(), &lll_uav_view_desc, CpuHandle);

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

    // Node table
    CpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(SrvHeap->GetCPUDescriptorHandleForHeapStart());
    GpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(SrvHeap->GetGPUDescriptorHandleForHeapStart());
    CpuHandle.Offset(SrvCounter*mCbvSrvUavDescriptorSize);
    GpuHandle.Offset(SrvCounter*mCbvSrvUavDescriptorSize);
    SrvCounter++;

    NodeTableHandle = GpuHandle;

    CD3DX12_RESOURCE_DESC NodeDesc(D3D12_RESOURCE_DIMENSION_BUFFER, 0, (1024) * sizeof(TempNode), 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE);
	NodeDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, 0, 0),
		D3D12_HEAP_FLAG_NONE,
		&NodeDesc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&NodeTable)
    );

	lll_uav_view_desc.Buffer.StructureByteStride = sizeof(TempNode); //2 uint32s in struct
	md3dDevice->CreateUnorderedAccessView(NodeTable.Get(), NodeTableCounter.Get(), &lll_uav_view_desc, CpuHandle);
    NodeTable->SetName(L"NodeTable");

    // light data look up table
    vector<TempLight> lights;
    TempLight l0;
    l0.id = 1;
    l0.pos = glm::vec3(1999, 12, 22);
    l0.radiance = 10.0;
    lights.push_back(l0);
    unsigned int byteSize = sizeof(TempLight) * lights.size();

    LightTable = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), lights.data(), byteSize, LightUploadBuffer);
    LightTable->SetName(L"LightTable");
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
	CD3DX12_ROOT_PARAMETER slotRootParameter[5];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &uavTable0);
	slotRootParameter[1].InitAsDescriptorTable(1, &uavTable1);
	slotRootParameter[2].InitAsDescriptorTable(1, &uavTable2);
    slotRootParameter[3].InitAsDescriptorTable(1, &depthTable);
    slotRootParameter[4].InitAsShaderResourceView(1);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter,
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
    // clear first, then compute
    // head, node counter
    unsigned int clearValue[4] = {0, 0, 0, 0};
    mCommandList->ClearUnorderedAccessViewUint(nullptr, nullptr, HeadTable.Get(), clearValue, 0, nullptr);

    // compute
    mCommandList->SetPipelineState(computePSO.Get());
    ID3D12DescriptorHeap* descriptorHeaps[] = { SrvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    mCommandList->SetComputeRootSignature(CSRootSignature.Get());
    mCommandList->SetComputeRootDescriptorTable(0, HeadTableHandle);
    mCommandList->SetComputeRootDescriptorTable(1, NodeTableHandle);
    mCommandList->Dispatch(1,1,1);

    mCommandList->SetPipelineState(debugPSO.Get());
    // ID3D12DescriptorHeap* descriptorHeaps[] = { SrvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    mCommandList->SetComputeRootSignature(CSRootSignature.Get());
    mCommandList->SetComputeRootDescriptorTable(0, HeadTableHandle);
    mCommandList->SetComputeRootDescriptorTable(1, NodeTableHandle);
    mCommandList->SetComputeRootDescriptorTable(2, DebugTableHandle);
    mCommandList->SetComputeRootDescriptorTable(3, clusterDepth->readHandle);
    mCommandList->SetComputeRootShaderResourceView(4, LightTable->GetGPUVirtualAddress());
    mCommandList->Dispatch(1, 1, 1);
}