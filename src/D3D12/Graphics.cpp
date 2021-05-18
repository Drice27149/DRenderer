#include "Graphics.hpp"
#include "DEngine.hpp"
#include "GraphicAPI.hpp"
#include "Struct.hpp"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

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
    // build it before texture loading
    BuildDescriptorHeaps();

    UploadMeshes();
    UploadTextures();

    InitPassMgrs();

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

	return true;
}

void Graphics::UploadMeshes()
{
    vector<Vertex> vs;
    vector<unsigned int> ids;
    for(Object* obj: DEngine::gobjs){
        for(Mesh mesh: obj->meshes){
            for(Vertex v: mesh.vs) vs.push_back(v);
            for(unsigned int id: mesh.ids) ids.push_back(id);
        }
    }

    objMesh = std::make_shared<DMesh>();
    objMesh->BuildVertexAndIndexBuffer(md3dDevice.Get(), mCommandList.Get(), vs, ids);
}

void Graphics::UploadTextures()
{
    textureMgr = std::make_shared<TextureMgr>(md3dDevice.Get(), mCommandList.Get());
    textureMgr->heapMgr = heapMgr;
    textureMgr->Init();
}

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
    shadowMgr = std::make_shared<ShadowMgr>(md3dDevice.Get(), mCommandList.Get(), 1024, 1024);
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
    skyBoxMgr = std::make_shared<SkyBoxMgr>(md3dDevice.Get(), mCommandList.Get(), srvCpu, srvGpu);
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

    lightCullMgr = std::make_shared<LightCullMgr>(md3dDevice.Get(), mCommandList.Get(), 16, 8, 4);
    for(int i = 0; i < 2; i++){
        heapMgr->GetNewSRV(srvCpu, srvGpu);
        lightCullMgr->srvCpu[i] = srvCpu;
        lightCullMgr->srvGpu[i] = srvGpu;
    }
    lightCullMgr->clusterDepth = clusterMgr->srvGpu;
    lightCullMgr->constantMgr = constantMgr;
    lightCullMgr->Init();

    // debug, for now cluster line only
    debugVisMgr = std::make_unique<DebugVisMgr>(md3dDevice.Get(), mCommandList.Get());
    debugVisMgr->offsetTable = lightCullMgr->srvGpu[0];
    debugVisMgr->entryTable = lightCullMgr->srvGpu[1];
    debugVisMgr->clusterDepth = clusterMgr->srvGpu;
    debugVisMgr->constantMgr = constantMgr;
    debugVisMgr->Init();

    // shading
    pbrMgr = std::make_unique<PBRMgr>(md3dDevice.Get(), mCommandList.Get());
    pbrMgr->constantMgr = constantMgr;
    pbrMgr->textureMgr = textureMgr;
    pbrMgr->lightCullMgr = lightCullMgr;
    pbrMgr->shadowMgr = shadowMgr;
    pbrMgr->objMesh = objMesh;
    pbrMgr->skyBoxMgr = skyBoxMgr;
    pbrMgr->Init();
    // gui
    guiMgr = std::make_unique<GUIMgr>(md3dDevice.Get(), mCommandList.Get());
    guiMgr->heapMgr = heapMgr;
    guiMgr->mhMainWnd = mhMainWnd;
    guiMgr->Init();
    GUIInit = true;
    // AA
    aaMgr = std::make_shared<AAMgr>(md3dDevice.Get(), mCommandList.Get());
    aaMgr->sWidth = 4*mClientWidth;
    aaMgr->sHeight = 4*mClientHeight;
    aaMgr->width = mClientWidth;
    aaMgr->height = mClientHeight;
    aaMgr->ssRate = ssRate;
    aaMgr->heapMgr = heapMgr;
    aaMgr->Init();
}

void Graphics::OnResize()
{
	D3DApp::OnResize();
}

void Graphics::Update(const GameTimer& gt)
{
    constantMgr->Update();
    guiMgr->Update();
}

void Graphics::Draw(const GameTimer& gt)
{
    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(constantMgr->frameResources[constantMgr->curFrame]->CmdListAlloc->Reset());
	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    ThrowIfFailed(mCommandList->Reset(constantMgr->frameResources[constantMgr->curFrame]->CmdListAlloc.Get(), nullptr));

    ID3D12DescriptorHeap* descriptorHeaps[] = { heapMgr->GetSRVHeap() };
    mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    DrawShadowMap();

    PreZPass();

    PrepareCluster();

    ExecuteComputeShader();

    aaMgr->PrePass();
    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    // Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &(aaMgr->rtvCpu), true, &(aaMgr->dsvCpu));
    // Clear the back buffer and depth buffer.
    mCommandList->ClearRenderTargetView(aaMgr->rtvCpu, Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearDepthStencilView(aaMgr->dsvCpu, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    mCommandList->RSSetViewports(1, &sScreenViewport);
    mCommandList->RSSetScissorRects(1, &sScissorRect);

    // real render
    // DrawObjects(DrawType::Normal);
    DrawOpaque();
    // place at last
    DrawSkyBox();
    // debugvis
    DrawLines();
    // GUI
    DrawGUI();

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Done recording commands.
	ThrowIfFailed(mCommandList->Close());
 
    // Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	
	// swap the back and front buffers
    // present 的时候, 并不能保证 backBuffer 已经渲染好了
    // 当 cpu 很快的时候是可以这样 assume 的, 行吧
    // TODO: fix this
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

    constantMgr->frameResources[constantMgr->curFrame]->Fence = ++mCurrentFence;

    mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void Graphics::BuildDescriptorHeaps()
{
    heapMgr = std::make_unique<HeapMgr>(md3dDevice.Get(), mCommandList.Get(), 50, 50, 50);
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

        for(Mesh& mesh: obj->meshes){
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

void Graphics::DrawOpaque()
{
    pbrMgr->PrePass();
    pbrMgr->Pass();
    pbrMgr->PostPass();
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

void Graphics::DrawGUI()
{
    guiMgr->Draw();
}
