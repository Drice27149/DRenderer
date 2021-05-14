#include "Graphics.hpp"
#include "DEngine.hpp"
#include "GraphicAPI.hpp"
#include "Struct.hpp"
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

    InitPassMgrs();

    BuildShadersAndInputLayout();

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
}

void Graphics::LoadAssets()
{
    // if(DEngine::gobj == nullptr) return ;
    // // Ŀǰ, ����ֻ��һ�� mesh
    // const Mesh& mesh = DEngine::gobj->meshes[0];
    // if (mesh.mask & (1 << aiTextureType_DIFFUSE)) {
    //     TTexture baseColorTex;
    //     CreateTextureFromImage(mesh.texns[aiTextureType_DIFFUSE], baseColorTex.Resource, baseColorTex.UploadHeap);
    //     textures.push_back(baseColorTex);
    // }
    // if (mesh.mask & (1 << aiTextureType_NORMALS)) {
    //     TTexture normalTex;
    //     CreateTextureFromImage(mesh.texns[aiTextureType_NORMALS], normalTex.Resource, normalTex.UploadHeap);
    //     textures.push_back(normalTex);
    // }
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
    debugVisMgr->PrePass();

    DrawObjects(DrawType::WhiteLines);
}

void Graphics::PrepareCluster()
{
    clusterMgr->PrePass();
    DrawObjects(DrawType::PointLight);
    clusterMgr->PostPass();
}   

void Graphics::ExecuteComputeShader()
{
    lightCullMgr->PrePass();
    lightCullMgr->Pass();
    lightCullMgr->PostPass();
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
    lightCullMgr->constantMgr = constantMgr;
    lightCullMgr->Init();

    debugVisMgr = std::make_unique<DebugVisMgr>(md3dDevice.Get(), mCommandList.Get());
    debugVisMgr->offsetTable = lightCullMgr->srvGpu[0];
    debugVisMgr->entryTable = lightCullMgr->srvGpu[1];
    debugVisMgr->clusterDepth = clusterMgr->srvGpu;
    debugVisMgr->constantMgr = constantMgr;
    debugVisMgr->Init();
}