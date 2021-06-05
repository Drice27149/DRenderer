#include "Graphics.hpp"
#include "DEngine.hpp"
#include "GraphicAPI.hpp"
#include "Struct.hpp"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "Fatory.hpp"
#include "Device.hpp"
#include "Context.hpp"
#include "Renderer.hpp"

const int FrameCount = 2;

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
    renderer = new Renderer();
    renderer->InitRenderer();
		
    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
    GDevice = md3dDevice.Get();
    GCmdList = mCommandList.Get();

    Device::SetDevice(md3dDevice);
    Context::SetContext(mCommandList);
    viewPortWidth = mClientWidth;
    viewPortHeight = mClientHeight;

    // build it before texture loading
    BuildDescriptorHeaps();
    UploadMeshes();
    UploadTextures();
    CreatePersistentResource();
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
    Renderer::ResManager->LoadObjectTextures();
}

void Graphics::CreatePersistentResource()
{
    Renderer::ResManager->CreateRenderTarget(
        std::string("DiffuseMetallic"),
        ResourceDesc {
            (unsigned int)mClientWidth,
            (unsigned int)mClientHeight,
            ResourceEnum::Format::R8G8B8A8_UNORM,
            ResourceEnum::Type::Texture2D,
        },
        1<<ResourceEnum::SRView | 1<<ResourceEnum::RTView
    );

    Renderer::ResManager->CreateRenderTarget(
        std::string("NormalRoughness"),
        ResourceDesc {
            (unsigned int)mClientWidth,
            (unsigned int)mClientHeight,
            ResourceEnum::Format::R8G8B8A8_UNORM,
            ResourceEnum::Type::Texture2D,
        },
        1<<ResourceEnum::SRView | 1<<ResourceEnum::RTView
    );

    Renderer::ResManager->CreateRenderTarget(
        std::string("WorldPosAO"),
        ResourceDesc {
            (unsigned int)mClientWidth,
            (unsigned int)mClientHeight,
            ResourceEnum::Format::R32G32B32A32_FLOAT,
            ResourceEnum::Type::Texture2D,
        },
        1<<ResourceEnum::SRView | 1<<ResourceEnum::RTView
    );

    Renderer::ResManager->CreateRenderTarget(
        std::string("Velocity"),
        ResourceDesc {
            (unsigned int)mClientWidth,
            (unsigned int)mClientHeight,
            ResourceEnum::Format::R16G16_FLOAT,
            ResourceEnum::Type::Texture2D,
        },
        1<<ResourceEnum::SRView | 1<<ResourceEnum::RTView
    );

    Renderer::ResManager->CreateRenderTarget(
        std::string("dummyTexture"),
        ResourceDesc {
            (unsigned int)mClientWidth,
            (unsigned int)mClientHeight,
            ResourceEnum::Format::R32G32B32A32_FLOAT,
            ResourceEnum::Type::Texture2D,
        },
        1<<ResourceEnum::SRView | 1<<ResourceEnum::RTView
    );

    Renderer::ResManager->CreateDepthStencil(
        std::string("GBufferDepth"),
        ResourceDesc{
            (unsigned int)mClientWidth,
            (unsigned int)mClientHeight,
            ResourceEnum::Format::R8G8B8A8_UNORM, // will be ignored
            ResourceEnum::Type::Texture2D,
        },
        1<<ResourceEnum::DSView
    );

    int dummyConstant = -1;
    Renderer::GDevice->SetShaderConstant(std::string("dummyConstant"), &dummyConstant);
}

void Graphics::InitPassMgrs()
{
    // 常量 (uniform) 管理类
    // @TODO: pass count 准确化
    // @TODO: Mgr 共享context
    // @TODO: 资源管理放在 Mgr
    ID3D12Device* device = md3dDevice.Get();
    ID3D12Fence* fence = mFence.Get();
    constantMgr = std::make_shared<ConstantMgr>(device, fence, (unsigned int)FrameCount, (unsigned int)5, (unsigned int)DEngine::gobjs.size());
    constantMgr->viewPortWidth = mClientWidth;
    constantMgr->viewPortHeight = mClientHeight;

    // gui
    guiMgr = std::make_shared<GUIMgr>(md3dDevice.Get(), mCommandList.Get());
    guiMgr->heapMgr = heapMgr;
    guiMgr->mhMainWnd = mhMainWnd;
    guiMgr->constantMgr = constantMgr;
    guiMgr->Init();
    GUIInit = true;

    return ;
    // Pre-Z 管理类
    preZMgr = std::make_shared<PreZMgr>(md3dDevice.Get(), mCommandList.Get(), 1024, 1024);
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
    skyBoxMgr = std::make_shared<SkyBoxMgr>(md3dDevice.Get(), mCommandList.Get());
    skyBoxMgr->Init();

    // light culling step 0: generate depth
    heapMgr->GetNewSRV(srvCpu, srvGpu);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE rtvGpu;
    heapMgr->GetNewRTV(rtvCpu, rtvGpu);
    clusterMgr = std::make_shared<ClusterMgr>(md3dDevice.Get(), mCommandList.Get(), 16, 8, 4);
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
    debugVisMgr = std::make_shared<DebugVisMgr>(md3dDevice.Get(), mCommandList.Get());
    debugVisMgr->offsetTable = lightCullMgr->srvGpu[0];
    debugVisMgr->entryTable = lightCullMgr->srvGpu[1];
    debugVisMgr->clusterDepth = clusterMgr->srvGpu;
    debugVisMgr->constantMgr = constantMgr;
    debugVisMgr->Init();

    // shading
    pbrMgr = std::make_shared<PBRMgr>(md3dDevice.Get(), mCommandList.Get());
    pbrMgr->objMesh = objMesh;
    pbrMgr->width = mClientWidth;
    pbrMgr->height = mClientHeight;
    pbrMgr->Init();

    // AA
    aaMgr = std::make_shared<AAMgr>(md3dDevice.Get(), mCommandList.Get());
    aaMgr->sWidth = ssRate*mClientWidth;
    aaMgr->sHeight = ssRate*mClientHeight;
    aaMgr->width = mClientWidth;
    aaMgr->height = mClientHeight;
    aaMgr->ssRate = ssRate;
    aaMgr->heapMgr = heapMgr;
    aaMgr->Init();
    // taa
    temporalAA = std::make_shared<TemporalAA>();
    temporalAA->inputs.resize(3);
    temporalAA->Init();
    // tone map
    toneMapping = std::make_shared<ToneMapping>();
    toneMapping->inputs.resize(2);
    toneMapping->Init();
    // bloom
    bloom = std::make_shared<Bloom>();
    bloom->inputs.resize(2);
    bloom->Init();
}

void Graphics::OnResize()
{
	D3DApp::OnResize();
}

void Graphics::Update(const GameTimer& gt)
{
    UpdateAndAsync();
}

void Graphics::UpdateAndAsync()
{
    constantMgr->Update();
    guiMgr->Update();
}

void Graphics::AddGBufferMainPass()
{
    // post processing code sample
    // Renderer::FG->AddPass(std::string("TestPass"),
    // [&](PassData& data){
    //     data.inputs = {
    //         ResourceData{std::string("testInfo"), ResourceEnum::State::Read, ResourceEnum::Type::Constant},
    //         ResourceData{"testSRV", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D},
    //     };
    //     data.outputs = {
    //         // ResourceData{"testCreate"},
    //     };
    //     data.psoData.enableDepth = false;
    //     // data.psoData.depthStencil = ...
    //     data.shaders = {
    //         ShaderData{std::string("../assets/shaders/test/pure.hlsl"), ShaderEnum::PS},
    //         ShaderData{std::string("../assets/shaders/test/pure.hlsl"), ShaderEnum::VS},
    //     };
    //     float color = 0.5;
    //     Renderer::GDevice->SetShaderConstant("testInfo", &color);
    // },
    // [=](){
    //     Renderer::GContext->GetContext()->DrawInstanced(6, 1, 0, 0);
    // });
    Renderer::FG->AddPass("GBufferMainPass",
        [&](PassData& data){
            data.inputs = {
                ResourceData{"dummyConstant", ResourceEnum::State::Read, ResourceEnum::Type::Constant}, // per object constant
                ResourceData{"dummyConstant", ResourceEnum::State::Read, ResourceEnum::Type::Constant}, // per pass constant
                ResourceData{"dummyTexture", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D }, // normal
                ResourceData{"dummyTexture", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D }, // baseColor
                ResourceData{"dummyTexture", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D }, // metallicRoughness
                ResourceData{"dummyTexture", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D }, // emissive
            };
            data.outputs = {
                ResourceData{"DiffuseMetallic", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R32G32B32A32_FLOAT},
                ResourceData{"NormalRoughness", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R32G32B32A32_FLOAT },
                ResourceData{"WorldPosAO", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R32G32B32A32_FLOAT },
                ResourceData{"Velocity", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R16G16_FLOAT },
            };
            data.psoData = PSOData{  
                true,   // enable depth 
                ResourceData{ "GBufferDepth", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D}, // depth stencil data
                (int)mClientWidth,
                (int)mClientHeight,
            };
            data.shaders = { 
                ShaderData{std::string("../assets/shaders/DeferredShading/GBufferMainPass.hlsl"), ShaderEnum::VS},
                ShaderData{std::string("../assets/shaders/DeferredShading/GBufferMainPass.hlsl"), ShaderEnum::PS},
            };
        },
        [=](){
            
        }
    );
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
    
    AddGBufferMainPass();

    // if(firstFrame){
    //     PrecomputeResource();
    // }

    // // DrawShadowMap();

    // // PreZPass();

    // // PrepareCluster();

    // // ExecuteComputeShader();
    
    // // begin of rendering a scene
    
    // aaMgr->BeginFrame();

    // CD3DX12_CPU_DESCRIPTOR_HANDLE rtvs[2] = {aaMgr->GetCurRTRTV(), pbrMgr->GetVelocityRTV()};

	// mCommandList->OMSetRenderTargets(2, rtvs, false, &(aaMgr->GetDepthBuffer()));
    // mCommandList->ClearRenderTargetView(aaMgr->GetCurRTRTV(), Colors::LightSteelBlue, 0, nullptr);
    // float clearColor[4] = {0.0, 0.0, 0.0, 0.0};
    // mCommandList->ClearRenderTargetView(pbrMgr->GetVelocityRTV(), clearColor, 0, nullptr);
    // mCommandList->ClearDepthStencilView(aaMgr->GetDepthBuffer(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    // mCommandList->RSSetViewports(1, &sScreenViewport);
    // mCommandList->RSSetScissorRects(1, &sScissorRect);

    // // // DrawObjects(DrawType::Normal);
    // DrawOpaque();
    // // // place at last
    // DrawSkyBox();
    // // // debugvis
    // // DrawLines();

    // // end of rendering a scene

    // // rtv -> srv, scroll it 
    // aaMgr->StartTAA();
    // // must connect with the last one
    // mCommandList->OMSetRenderTargets(1, &(aaMgr->GetNextRenderTarget()), true, &(aaMgr->GetDepthBuffer()));

    // if(constantMgr->GetSceneInfo()->taa){
    //     temporalAA->PrePass();
    //     temporalAA->Pass();
    //     temporalAA->PostPass();
    // }

    // aaMgr->EndTAA();

    // if (constantMgr->GetSceneInfo()->taa)
    //     bloom->input = aaMgr->GetTAAResult();
    // else
    //     bloom->input = aaMgr->GetCurRTSRV();
    // bloom->BloomPass();

    // // draw to screen
    // // transition first
    // mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    // // change render target
    // mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
    // mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
    // mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    // mCommandList->RSSetViewports(1, &mScreenViewport);
    // mCommandList->RSSetScissorRects(1, &mScissorRect);

    // if(constantMgr->GetSceneInfo()->taa)
    //     toneMapping->input = aaMgr->GetTAAResult();
    // else 
    //     toneMapping->input = aaMgr->GetCurRTSRV();

    // Renderer::ResManager->RegisterHandle(std::string("testTarget"), CD3DX12_CPU_DESCRIPTOR_HANDLE(CurrentBackBufferView()), ResourceEnum::View::RTView);
    // Renderer::ResManager->RegisterHandle(std::string("testSRV"), aaMgr->GetTAAResult(), ResourceEnum::View::SRView);

    // if(firstFrame){
        
    //     firstFrame = false;
    // }
    

    // Renderer::FG->AddPass(std::string("TestPass"),
    // [&](PassData& data){
    //     data.inputs = {
    //         ResourceData{std::string("testInfo"), ResourceEnum::State::Read, ResourceEnum::Type::Constant},
    //         ResourceData{"testSRV", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D},
    //     };
    //     data.outputs = {
    //         // ResourceData{"testCreate"},
    //     };
    //     data.psoData.enableDepth = false;
    //     // data.psoData.depthStencil = ...
    //     data.shaders = {
    //         ShaderData{std::string("../assets/shaders/test/pure.hlsl"), ShaderEnum::PS},
    //         ShaderData{std::string("../assets/shaders/test/pure.hlsl"), ShaderEnum::VS},
    //     };
    //     float color = 0.5;
    //     Renderer::GDevice->SetShaderConstant("testInfo", &color);
    // },
    // [=](){
    //     Renderer::GContext->GetContext()->DrawInstanced(6, 1, 0, 0);
    // });
    // // Renderer::FG->AddPass(name, name, name);

    // // toneMapping->PrePass();
    // // toneMapping->Pass();
    // // toneMapping->PostPass();
    // // GUI
    // DrawGUI();

    // only hard code work here
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
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
    heapMgr = std::make_unique<HeapMgr>(md3dDevice.Get(), mCommandList.Get(), 100, 100, 100);
}

void Graphics::PrecomputeResource()
{
    if (prefilterIBL == nullptr) {
        prefilterIBL = std::make_shared<PrefilterIBL>();
        prefilterIBL->rootors = {
            RootEntryFatory::CBVEntry(0),
            RootEntryFatory::SRVEntry(skyBoxMgr->GetCubeMapSrv())
        };
        prefilterIBL->Init();
    }
    prefilterIBL->PreComputeFilter();
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

void Graphics::DrawGUI()
{
    guiMgr->Draw();
}

