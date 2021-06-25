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
#include "ViewFatory.hpp"
#include "Mipmap.hpp"

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
    GeneratePrimitive();
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
    // dummy constant and dummy texture
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

    int dummyConstant = -1;
    Renderer::GDevice->SetShaderConstant(std::string("dummyConstant"), &dummyConstant, true);

    // GBuffer
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
            ResourceEnum::Format::R32G32B32A32_FLOAT,
            ResourceEnum::Type::Texture2D,
        },
        1<<ResourceEnum::SRView | 1<<ResourceEnum::RTView
    );

    Renderer::ResManager->CreateRenderTarget(
        std::string("WorldPosX"),
        ResourceDesc {
            (unsigned int)mClientWidth,
            (unsigned int)mClientHeight,
            ResourceEnum::Format::R32G32B32A32_FLOAT,
            ResourceEnum::Type::Texture2D,
        },
        1<<ResourceEnum::SRView | 1<<ResourceEnum::RTView
    );

    Renderer::ResManager->CreateRenderTarget(
        std::string("ViewPosY"),
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
        std::string("ColorBuffer"),
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

    Renderer::ResManager->CreateRenderTarget(
        std::string("PostProcessBuffer"),
        ResourceDesc{
            (unsigned int)mClientWidth,
            (unsigned int)mClientHeight,
            ResourceEnum::Format::R32G32B32A32_FLOAT, // will be ignored
            ResourceEnum::Type::Texture2D,
        },
        1<<ResourceEnum::SRView | 1<<ResourceEnum::RTView
    );

    Renderer::ResManager->CreateRenderTarget(
        std::string("HistoryBuffer"),
        ResourceDesc{
            (unsigned int)mClientWidth,
            (unsigned int)mClientHeight,
            ResourceEnum::Format::R32G32B32A32_FLOAT, // will be ignored
            ResourceEnum::Type::Texture2D,
        },
        1<<ResourceEnum::SRView | 1<<ResourceEnum::RTView
    );

    Renderer::ResManager->CreateDepthStencil(
        std::string("ShadowDepth"),
        ResourceDesc{
            (unsigned int)2048,
            (unsigned int)2048,
            ResourceEnum::Format::R32G32B32A32_FLOAT, // will be ignored
            ResourceEnum::Type::Texture2D,
        },
        1<<ResourceEnum::SRView | 1<<ResourceEnum::DSView
    );

    Renderer::ResManager->CreateRenderTarget(
        std::string("ShadowMap"),
        ResourceDesc{
            (unsigned int)2048,
            (unsigned int)2048,
            ResourceEnum::Format::R32G32B32A32_FLOAT, // will be ignored
            ResourceEnum::Type::Texture2D,
        },
        1<<ResourceEnum::SRView | 1<<ResourceEnum::RTView
    );

    // voxel grid
    Renderer::ResManager->CreateTexture3D(
        std::string("VoxelGridR"),
        ResourceDesc {
            (unsigned int)voxelX, 
            (unsigned int)voxelY,
            ResourceEnum::Format::R32_UINT,
            ResourceEnum::Type::Texture3D,
            ResourceEnum::State::Write
        },
        (unsigned int)voxelZ,
        1<<ResourceEnum::UAView// | 1<<ResourceEnum::SRView
    );

    auto res = Renderer::ResManager->GetResource("VoxelGridR");
    heapMgr->GetNewUAV(cpuVoxel, gpuVoxel);
    ViewFatory::AppendUAV(res, res->GetDesc().Format, D3D12_UAV_DIMENSION_TEXTURE3D, cpuVoxel);

    Renderer::ResManager->CreateTexture3D(
        std::string("VoxelGridG"),
        ResourceDesc {
            (unsigned int)voxelX, 
            (unsigned int)voxelY,
            ResourceEnum::Format::R32_UINT,
            ResourceEnum::Type::Texture3D,
            ResourceEnum::State::Write
        },
        (unsigned int)voxelZ,
        1<<ResourceEnum::UAView// | 1<<ResourceEnum::SRView
    );

    res = Renderer::ResManager->GetResource("VoxelGridG");
    heapMgr->GetNewUAV(cpuVoxel, gpuVoxel);
    ViewFatory::AppendUAV(res, res->GetDesc().Format, D3D12_UAV_DIMENSION_TEXTURE3D, cpuVoxel);

    Renderer::ResManager->CreateTexture3D(
        std::string("VoxelGridB"),
        ResourceDesc {
            (unsigned int)voxelX, 
            (unsigned int)voxelY,
            ResourceEnum::Format::R32_UINT,
            ResourceEnum::Type::Texture3D,
            ResourceEnum::State::Write
        },
        (unsigned int)voxelZ,
        1<<ResourceEnum::UAView// | 1<<ResourceEnum::SRView
    );

    res = Renderer::ResManager->GetResource("VoxelGridB");
    heapMgr->GetNewUAV(cpuVoxel, gpuVoxel);
    ViewFatory::AppendUAV(res, res->GetDesc().Format, D3D12_UAV_DIMENSION_TEXTURE3D, cpuVoxel);

    Renderer::ResManager->CreateTexture3D(
        std::string("VoxelGridA"),
        ResourceDesc {
            (unsigned int)voxelX, 
            (unsigned int)voxelY,
            ResourceEnum::Format::R32_UINT,
            ResourceEnum::Type::Texture3D,
            ResourceEnum::State::Write
        },
        (unsigned int)voxelZ,
        1<<ResourceEnum::UAView// | 1<<ResourceEnum::SRView
    );

    res = Renderer::ResManager->GetResource("VoxelGridA");
    heapMgr->GetNewUAV(cpuVoxel, gpuVoxel);
    ViewFatory::AppendUAV(res, res->GetDesc().Format, D3D12_UAV_DIMENSION_TEXTURE3D, cpuVoxel);

    Mipmap::CreateMipmap(voxelX);
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
    guiMgr->mhMainWnd = mhMainWnd;
    guiMgr->Init();
    GUIInit = true;
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
    acFrame++;
    Renderer::ResManager->ForwardFrame();
    if(acFrame>=4)
        Renderer::ResManager->ForwardResource();
    constantMgr->Update();
    guiMgr->Update();
}

void Graphics::AddGBufferMainPass()
{
    Renderer::FG->AddPass("GBufferMainPass",
        [&](PassData& data){
            data.inputs = {
                ResourceData{"dummyConstant", ResourceEnum::State::Read, ResourceEnum::Type::Constant}, // per object constant
                ResourceData{"dummyConstant", ResourceEnum::State::Read, ResourceEnum::Type::Constant}, // per pass constant
                ResourceData{"dummyTexture", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D }, // normal
                ResourceData{"dummyTexture", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D }, // baseColor
                ResourceData{"dummyTexture", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D }, // metallicRoughness
                ResourceData{"dummyTexture", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D }, // emissive
                ResourceData{"dummyTexture", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D }, // height map
            };
            data.outputs = {
                ResourceData{"DiffuseMetallic", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R8G8B8A8_UNORM},
                ResourceData{"NormalRoughness", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R32G32B32A32_FLOAT},
                ResourceData{"WorldPosX", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R32G32B32A32_FLOAT },
                ResourceData{"ViewPosY", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R32G32B32A32_FLOAT },
                ResourceData{"Velocity", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R16G16_FLOAT },
            };
            data.psoData = PSOData{  
                true,   // enable depth 
                false,
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
            // @TODO: clear RT and depth here
            // @TODO: constant commit and done
            Context::GetContext()->IASetVertexBuffers(0, 1, &objMesh->VertexBufferView());
            Context::GetContext()->IASetIndexBuffer(&objMesh->IndexBufferView());
            Context::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // clear gbuffer and depth stencil
            float clearC[4] = {0.0, 0.0, 0.0, 0.0};
            vector<std::string> clearTargets = {"DiffuseMetallic", "NormalRoughness", "WorldPosX", "ViewPosY", "Velocity"};
            for(auto& target: clearTargets){
                auto rtHandle = Renderer::ResManager->GetCPU(target, ResourceEnum::View::RTView);
                Context::GetContext()->ClearRenderTargetView(rtHandle, clearC, 0, nullptr);
            }
            auto dsvHandle = Renderer::ResManager->GetCPU("GBufferDepth", ResourceEnum::View::DSView);
            Context::GetContext()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

            int mapping[30];
            for(int i = 0; i < 25; i++)
                mapping[i] = -1;
            mapping[aiTextureType_NORMALS] = 2;
            mapping[aiTextureType_DIFFUSE] = 3;
            mapping[aiTextureType_LIGHTMAP] = 4;
            mapping[aiTextureType_EMISSIVE] = 5;
            mapping[aiTextureType_METALNESS] = 4;
            mapping[aiTextureType_DIFFUSE_ROUGHNESS] = 4;
            mapping[aiTextureType_HEIGHT] = 6;
            mapping[aiTextureType_UNKNOWN] = 4;

            auto objectAddr = Graphics::constantMgr->GetObjectConstant((unsigned long long)0);
            int idOffset = 0, vsOffset = 0;
            
            auto passAddr = Graphics::constantMgr->GetCameraPassConstant();
            Context::GetContext()->SetGraphicsRootConstantBufferView(0, passAddr);

            for(Object* obj: DEngine::gobjs){
                Context::GetContext()->SetGraphicsRootConstantBufferView(1, objectAddr);
                // draw per mesh
                for(Mesh& mesh: obj->meshes){
                    // set up material and texture
                    for(int i = 0; i < aiTextureType_UNKNOWN+1; i++){
                        if(mesh.mask & (1<<i)){
                            unsigned int slot = mapping[i];
                            if(slot != -1){
                                auto& handle = Renderer::ResManager->GetGPU(mesh.texns[i], ResourceEnum::View::SRView);
                                Context::GetContext()->SetGraphicsRootDescriptorTable(slot, handle);
                            }
                        }
                    }
                    // issue the draw call
                    int idSize = mesh.ids.size();
                    if(obj->drawType == DrawType::Normal) 
                        Context::GetContext()->DrawIndexedInstanced(idSize, 1, idOffset, vsOffset, 0);
                    idOffset += mesh.ids.size();
                    vsOffset += mesh.vs.size();
                }
                objectAddr += d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectUniform));
            }
        }
    );
}

void Graphics::AddLightPass()
{
    // post processing code sample
    Renderer::FG->AddPass(std::string("MainLightPass"),
    [&](PassData& data){
        data.inputs = {
            ResourceData{ "dummyConstant", ResourceEnum::State::Read, ResourceEnum::Type::Constant },
            ResourceData{ "dummyConstant", ResourceEnum::State::Read, ResourceEnum::Type::Constant },
            ResourceData{ "DiffuseMetallic", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D },
            ResourceData{ "NormalRoughness", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D },
            ResourceData{ "WorldPosX", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D },
            ResourceData{ "ViewPosY", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D },
            ResourceData{ "ShadowMap", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D },
            ResourceData{ "EnvRadiance", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D },
            ResourceData{ "EnvLUT", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D },
        };
        data.outputs = {
            ResourceData{ "ColorBuffer", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R32G32B32A32_FLOAT } // will use CurrentBackBuffer()
        };
        data.psoData = PSOData {
            false,   // enable depth 
            true,
            ResourceData{},
            (int)mClientWidth,
            (int)mClientHeight,
        };
        data.shaders = {
            ShaderData{ std::string("../assets/shaders/DeferredShading/lightPass.hlsl"), ShaderEnum::PS },
            ShaderData{ std::string("../assets/shaders/DeferredShading/lightPass.hlsl"), ShaderEnum::VS },
        };
    },
    [=](){
        float clearColor[4] = {0.0, 0.0, 0.0, 0.0};
        auto colorBufferHandle = Renderer::ResManager->GetCPU("ColorBuffer", ResourceEnum::View::RTView);
        Context::GetContext()->ClearRenderTargetView(colorBufferHandle, clearColor, 0, nullptr);

        struct LightDesc {
            float f[4]; // directional light: dx, dy, dz, intensity
            int sh;
        };
        LightDesc l0[4];
        l0[0] = LightDesc {1.0, 1.0, 1.0, 1.0, 1};
        Renderer::GDevice->SetShaderConstant("LightSource0", &(l0[0]));
        l0[1] = LightDesc {-1.0, 1.0, 1.0, 1.0, 1};
        Renderer::GDevice->SetShaderConstant("LightSource1", &(l0[1]));
        l0[2] = LightDesc {0.0, 1.0, 1.0, 0.4, 0};
        Renderer::GDevice->SetShaderConstant("LightSource2", &(l0[2]));
        l0[3] = LightDesc {0.0, 1.0, -1.0, 0.4, 0};
        Renderer::GDevice->SetShaderConstant("LightSource2", &(l0[3]));

        auto passAddr = Graphics::constantMgr->GetCameraPassConstant();
        Context::GetContext()->SetGraphicsRootConstantBufferView(1, passAddr);

        for(int i = 1; i < 2; i++){
            std::string resName = "LightSource";
            resName.push_back('0'+i);
            auto lightRes = Renderer::ResManager->GetResource(resName);
            Renderer::GContext->GetContext()->SetGraphicsRootConstantBufferView(0, lightRes->GetGPUVirtualAddress());
            Renderer::GContext->GetContext()->DrawInstanced(6, 1, 0, 0);
        }
    });
}

void Graphics::AddCopyPass(std::string from, std::string to)
{
    Renderer::FG->AddPass(std::string(from+"->"+to),
    [&](PassData& data){
        data.inputs = {
            ResourceData{ from, ResourceEnum::State::Read, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R32G32B32A32_FLOAT } 
        };
        data.outputs = {
            ResourceData{ to, ResourceEnum::State::Write, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R32G32B32A32_FLOAT } 
        };
        data.psoData = PSOData {
            false,   // enable depth 
            true,
            ResourceData{},
            (int)mClientWidth,
            (int)mClientHeight,
        };
        data.shaders = {
            ShaderData{ std::string("../assets/shaders/DeferredShading/CopyTexture.hlsl"), ShaderEnum::PS },
            ShaderData{ std::string("../assets/shaders/DeferredShading/CopyTexture.hlsl"), ShaderEnum::VS },
        };
    },
    [=](){
        float clearColor[4] = {0.0, 0.0, 0.0, 0.0};
        auto colorBufferHandle = Renderer::ResManager->GetCPU(to, ResourceEnum::View::RTView);
        Context::GetContext()->ClearRenderTargetView(colorBufferHandle, clearColor, 0, nullptr);

        Renderer::GContext->GetContext()->DrawInstanced(6, 1, 0, 0);
    });
}

void Graphics::AddPostProcessPass()
{
    // post processing code sample
    Renderer::FG->AddPass(std::string("PresentToScreen"),
    [&](PassData& data){
        data.inputs = {
            ResourceData{"PostProcessBuffer", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R32G32B32A32_FLOAT },
        };
        data.outputs = {
            ResourceData{"dummyTexture", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D } // will use CurrentBackBuffer()
        };
        data.psoData = PSOData{  
            false,   // enable depth 
            false,
            ResourceData{},
            (int)mClientWidth,
            (int)mClientHeight,
        };
        data.shaders = {
            ShaderData{std::string("../assets/shaders/DeferredShading/ToneMapping.hlsl"), ShaderEnum::PS},
            ShaderData{std::string("../assets/shaders/DeferredShading/ToneMapping.hlsl"), ShaderEnum::VS},
        };
    },
    [=](){
        Context::GetContext()->OMSetRenderTargets(1, &CurrentBackBufferView(), false, nullptr);
        float clearColor[4] = {0.0, 0.0, 0.0, 0.0};
        Context::GetContext()->ClearRenderTargetView(CurrentBackBufferView(), clearColor, 0, nullptr);

        Renderer::GContext->GetContext()->DrawInstanced(6, 1, 0, 0);
    });
}

void Graphics::AddTAAPass()
{
    // post processing code sample
    Renderer::FG->AddPass(std::string("TemporalAA"),
    [&](PassData& data){
        data.inputs = {
            ResourceData{"ColorBuffer", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R32G32B32A32_FLOAT },
            ResourceData{"HistoryBuffer", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R32G32B32A32_FLOAT },
            ResourceData{"Velocity", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R16G16_FLOAT },
        };
        data.outputs = {
            ResourceData{"PostProcessBuffer", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R32G32B32A32_FLOAT } // will use CurrentBackBuffer()
        };
        data.psoData = PSOData{  
            false,   // enable depth 
            false,
            ResourceData{},
            (int)mClientWidth,
            (int)mClientHeight,
        };
        data.shaders = {
            ShaderData{std::string("../assets/shaders/DeferredShading/TemporalAA.hlsl"), ShaderEnum::PS},
            ShaderData{std::string("../assets/shaders/DeferredShading/TemporalAA.hlsl"), ShaderEnum::VS},
        };
    },
    [=](){
        float clearColor[4] = {0.0, 0.0, 0.0, 0.0};
        auto colorBufferHandle = Renderer::ResManager->GetCPU("PostProcessBuffer", ResourceEnum::View::RTView);
        Context::GetContext()->ClearRenderTargetView(colorBufferHandle, clearColor, 0, nullptr);

        Renderer::GContext->GetContext()->DrawInstanced(6, 1, 0, 0);
    });
}

void Graphics::AddShadowPass()
{
    Renderer::FG->AddPass("ShadowPass",
        [&](PassData& data){
            data.inputs = {
                ResourceData{"dummyConstant", ResourceEnum::State::Read, ResourceEnum::Type::Constant}, // per object constant
                ResourceData{"dummyConstant", ResourceEnum::State::Read, ResourceEnum::Type::Constant}, // per pass constant
            };
            data.outputs = {
                ResourceData{"ShadowMap", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R32G32B32A32_FLOAT } // will use CurrentBackBuffer()
            };
            data.psoData = PSOData{  
                true,   // enable depth 
                false,
                ResourceData{ "ShadowDepth", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D}, // depth stencil data
                (int)2048,
                (int)2048,
            };
            data.shaders = { 
                ShaderData{std::string("../assets/shaders/DeferredShading/Shadow.hlsl"), ShaderEnum::VS},
                ShaderData{std::string("../assets/shaders/DeferredShading/Shadow.hlsl"), ShaderEnum::PS},
            };
        },
        [=](){
            // @TODO: clear RT and depth here
            // @TODO: constant commit and done
            Context::GetContext()->IASetVertexBuffers(0, 1, &objMesh->VertexBufferView());
            Context::GetContext()->IASetIndexBuffer(&objMesh->IndexBufferView());
            Context::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // clear depth stencil
            float clearC[4] = {0.0, 0.0, 0.0, 0.0};
            auto dsvHandle = Renderer::ResManager->GetCPU("ShadowDepth", ResourceEnum::View::DSView);
            Context::GetContext()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

            auto objectAddr = Graphics::constantMgr->GetObjectConstant((unsigned long long)0);
            int idOffset = 0, vsOffset = 0;
            
            auto passAddr = Graphics::constantMgr->GetCameraPassConstant();
            Context::GetContext()->SetGraphicsRootConstantBufferView(0, passAddr);

            for(Object* obj: DEngine::gobjs){
                Context::GetContext()->SetGraphicsRootConstantBufferView(1, objectAddr);
                // rendering
                for(Mesh& mesh: obj->meshes){
                    int idSize = mesh.ids.size();
                    if(obj->drawType == DrawType::Normal) 
                        Context::GetContext()->DrawIndexedInstanced(idSize, 1, idOffset, vsOffset, 0);

                    idOffset += mesh.ids.size();
                    vsOffset += mesh.vs.size();
                }
                objectAddr += d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectUniform));
            }
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
    
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	
    if(firstFrame){
         PrecomputeResource();
         firstFrame = false;
    }

    //AddShadowPass();
    //AddGBufferMainPass();
    //AddLightPass();
    VoxelizeScene(voxelX, voxelY, voxelZ);
    //MipmapVoxel(voxelX, voxelY, voxelZ);
    RenderVoxel(voxelX, voxelY, voxelZ);
    //DrawSkyBox();

    if(acFrame == 1)
        AddCopyPass("ColorBuffer", "HistoryBuffer");

    AddTAAPass();

    AddCopyPass("PostProcessBuffer", "HistoryBuffer");

    AddPostProcessPass();

    guiMgr->Draw();

    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

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
        
    //     
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
    skyBoxMgr = std::make_shared<SkyBoxMgr>(md3dDevice.Get(), mCommandList.Get());
    skyBoxMgr->Init();

    if (prefilterIBL == nullptr) {
        prefilterIBL = std::make_shared<PrefilterIBL>();
        prefilterIBL->rootors = {
            RootEntryFatory::CBVEntry(0),
            RootEntryFatory::SRVEntry(skyBoxMgr->GetCubeMapSrv())
        };
        prefilterIBL->Init();
    }
    prefilterIBL->PreComputeFilter();

    Renderer::ResManager->RegisterHandle("EnvRadiance", prefilterIBL->GetPrefilterEnvMap(), ResourceEnum::View::SRView);
    Renderer::ResManager->RegisterStateTrack("EnvRadiance", ResourceEnum::State::Read);
    Renderer::ResManager->RegisterTypeTrack("EnvRadiance", ResourceEnum::Type::Texture2D);
    Renderer::ResManager->RegisterHandle("EnvLUT", prefilterIBL->GetEnvBRDFMap(), ResourceEnum::View::SRView);
    Renderer::ResManager->RegisterStateTrack("EnvLUT", ResourceEnum::State::Read);
    Renderer::ResManager->RegisterTypeTrack("EnvLUT", ResourceEnum::Type::Texture2D);
}

void Graphics::VoxelizeScene(unsigned int x, unsigned int y, unsigned z)
{
    // post processing code sample
    Renderer::FG->AddPass(std::string("Voxelization"),
    [&](PassData& data){
        data.inputs = {
            ResourceData{"VoxelPassConstant", ResourceEnum::State::Read, ResourceEnum::Type::Constant}, // voxelization params
            ResourceData{"dummyConstant", ResourceEnum::State::Read, ResourceEnum::Type::Constant}, // pass constant
            ResourceData{"dummyConstant", ResourceEnum::State::Read, ResourceEnum::Type::Constant}, // per object constant
            ResourceData{"dummyTexture", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D }, // normal
            ResourceData{"dummyTexture", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D }, // baseColor
            ResourceData{"dummyTexture", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D }, // metallicRoughness
            ResourceData{"dummyTexture", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D }, // emissive
            ResourceData{"ShadowMap", ResourceEnum::State::Read, ResourceEnum::Type::Texture2D },
            ResourceData{"VoxelGridR", ResourceEnum::State::Write, ResourceEnum::Type::Texture3D},   // voxel grid, keep voxel data
            ResourceData{"VoxelGridG", ResourceEnum::State::Write, ResourceEnum::Type::Texture3D},   // voxel grid, keep voxel data
            ResourceData{"VoxelGridB", ResourceEnum::State::Write, ResourceEnum::Type::Texture3D},   // voxel grid, keep voxel data
            ResourceData{"VoxelGridA", ResourceEnum::State::Write, ResourceEnum::Type::Texture3D},   // voxel grid, keep voxel data
        };
        data.outputs = {
        };
        data.psoData = PSOData{  
            false,   // enable depth 
            false,
            ResourceData{},
            (int)x,
            (int)y,
            true,
        };
        data.shaders = {
            ShaderData{std::string("../assets/shaders/Voxel/Voxelize.hlsl"), ShaderEnum::PS},
            ShaderData{std::string("../assets/shaders/Voxel/Voxelize.hlsl"), ShaderEnum::GS},
            ShaderData{std::string("../assets/shaders/Voxel/Voxelize.hlsl"), ShaderEnum::VS},
        };

        // set up voxelization constant
        struct VoxelPass {
            int width;
            int height;
            int depth;
            int sizeX;
            int sizeY;
            int sizeZ;
            glm::mat4 ortho;
        };
        VoxelPass voxelPassConstant = VoxelPass {
            (int)x, // block count
            (int)y,
            (int)z,
            2048,   // actual length
            2048,
            2048,
        };
        Renderer::GDevice->SetShaderConstant("VoxelPassConstant", &voxelPassConstant);
    },
    [=](){
        Context::GetContext()->IASetVertexBuffers(0, 1, &objMesh->VertexBufferView());
        Context::GetContext()->IASetIndexBuffer(&objMesh->IndexBufferView());
        Context::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        // clear last frame voxel data first )
        std::string letter = "RGBA";
        for(int i = 0; i < 4; i++){
            std::string cName = "VoxelGrid";
            cName.push_back(letter[i]);
            auto voxelGridGpu = Renderer::ResManager->GetGPU(cName, ResourceEnum::View::UAView);
            auto voxelGridCpu = Renderer::ResManager->GetCPU(cName, ResourceEnum::View::UAView);
            auto voxelRes = Renderer::ResManager->GetResource(cName);
            float clearColor[4] = {0.0, 0.0, 0.0, 0.0};
            Renderer::GContext->GetContext()->ClearUnorderedAccessViewFloat(gpuVoxel, cpuVoxel, voxelRes, clearColor, 0, nullptr);
        }

        int mapping[30];
        for(int i = 0; i < 25; i++)
            mapping[i] = -1;
        mapping[aiTextureType_NORMALS] = 3;
        mapping[aiTextureType_DIFFUSE] = 4;
        mapping[aiTextureType_LIGHTMAP] = 5;
        mapping[aiTextureType_EMISSIVE] = 6;
        mapping[aiTextureType_METALNESS] = 5;
        mapping[aiTextureType_DIFFUSE_ROUGHNESS] = 5;

        auto objectAddr = Graphics::constantMgr->GetObjectConstant((unsigned long long)0);
        int idOffset = 0, vsOffset = 0;
        
        auto passAddr = Graphics::constantMgr->GetCameraPassConstant();
        Context::GetContext()->SetGraphicsRootConstantBufferView(1, passAddr);

        for(Object* obj: DEngine::gobjs){
            Context::GetContext()->SetGraphicsRootConstantBufferView(2, objectAddr);
            // draw per mesh
            for(Mesh& mesh: obj->meshes){
                // set up material and texture
                for(int i = 0; i < aiTextureType_UNKNOWN+1; i++){
                    if(mesh.mask & (1<<i)){
                        unsigned int slot = mapping[i];
                        if(slot != -1){
                            auto& handle = Renderer::ResManager->GetGPU(mesh.texns[i], ResourceEnum::View::SRView);
                            Context::GetContext()->SetGraphicsRootDescriptorTable(slot, handle);
                        }
                    }
                }
                // issue the draw call
                int idSize = mesh.ids.size();
                if(obj->drawType == DrawType::Normal) 
                    Context::GetContext()->DrawIndexedInstanced(idSize, 1, idOffset, vsOffset, 0);
                idOffset += mesh.ids.size();
                vsOffset += mesh.vs.size();
            }
            objectAddr += d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectUniform));
        }

        // barrier
        Renderer::GContext->GetContext()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(nullptr));
    });
}

void Graphics::RenderVoxel(unsigned int x, unsigned int y, unsigned int z)
{
    Renderer::FG->AddPass(std::string("RenderVoxel"),
    [&](PassData& data){
        data.inputs = {
            ResourceData{"VoxelPassConstant", ResourceEnum::State::Read, ResourceEnum::Type::Constant}, // voxelization params
            ResourceData{"dummyConstant", ResourceEnum::State::Read, ResourceEnum::Type::Constant}, // per pass constant
            ResourceData{"VoxelGridR", ResourceEnum::State::Write, ResourceEnum::Type::Texture3D},
            ResourceData{"VoxelGridG", ResourceEnum::State::Write, ResourceEnum::Type::Texture3D},
            ResourceData{"VoxelGridB", ResourceEnum::State::Write, ResourceEnum::Type::Texture3D},
            ResourceData{"VoxelGridA", ResourceEnum::State::Write, ResourceEnum::Type::Texture3D},
        };
        data.outputs = {
            ResourceData{ "ColorBuffer", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D, ResourceEnum::Format::R32G32B32A32_FLOAT }, // will use CurrentBackBuffer()
        };
        data.psoData = PSOData{  
            true,   // enable depth 
            false,  // blend add
            ResourceData{ "GBufferDepth", ResourceEnum::State::Write, ResourceEnum::Type::Texture2D}, // depth stencil data
            (int)mClientWidth,
            (int)mClientHeight,
            false,   // conservative
        };
        data.shaders = {
            ShaderData{std::string("../assets/shaders/Voxel/RenderVoxel.hlsl"), ShaderEnum::PS},
            ShaderData{std::string("../assets/shaders/Voxel/RenderVoxel.hlsl"), ShaderEnum::GS},
            ShaderData{std::string("../assets/shaders/Voxel/RenderVoxel.hlsl"), ShaderEnum::VS},
        };
    },
    [=](){
        Context::GetContext()->IASetVertexBuffers(0, 1, &voxelMesh->VertexBufferView());
        Context::GetContext()->IASetIndexBuffer(&voxelMesh->IndexBufferView());
        Context::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        float clearColor[4] = {0.0, 0.0, 0.0, 0.0};
        auto colorBufferHandle = Renderer::ResManager->GetCPU("ColorBuffer", ResourceEnum::View::RTView);
        Context::GetContext()->ClearRenderTargetView(colorBufferHandle, clearColor, 0, nullptr);

        auto dsvHandle = Renderer::ResManager->GetCPU("GBufferDepth", ResourceEnum::View::DSView);
        Context::GetContext()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

        auto passAddr = Graphics::constantMgr->GetCameraPassConstant();
        Context::GetContext()->SetGraphicsRootConstantBufferView(1, passAddr);

        Renderer::GContext->GetContext()->DrawIndexedInstanced(36, voxelX*voxelY*voxelZ, 0, 0, 0);

        // barrier
        Renderer::GContext->GetContext()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(nullptr));
    });
}

void Graphics::MipmapVoxel(unsigned int x, unsigned int y, unsigned int z)
{
    struct MipLevel {
        unsigned int level;
    };
    int length = x;
    int l = x;
    int count = 0;
    while(l!=0){
        l /= 2;
        count++;
    }

    for(int round = 0; round < count; round++){
        std::string passName = "MipmapVoxel";
        passName.push_back('0'+round);
        Renderer::FG->AddPass(passName,
        [&](PassData& data){
            data.inputs = {
                ResourceData{"MipConstant", ResourceEnum::State::Read, ResourceEnum::Type::Constant}, // mipmap constant
                ResourceData{"VoxelGridR", ResourceEnum::State::Write, ResourceEnum::Type::Texture3D},
                ResourceData{"VoxelGridG", ResourceEnum::State::Write, ResourceEnum::Type::Texture3D},
                ResourceData{"VoxelGridB", ResourceEnum::State::Write, ResourceEnum::Type::Texture3D},
                ResourceData{"VoxelGridA", ResourceEnum::State::Write, ResourceEnum::Type::Texture3D},
                ResourceData{"VoxelMip", ResourceEnum::State::Write, ResourceEnum::Type::Texture3D},    // transfer state && dummy for actual mip slice
                ResourceData{"VoxelMip", ResourceEnum::State::Write, ResourceEnum::Type::Texture3D},    // transfer state && dummy for actual mip slice
            };
            data.outputs = {
            };
            data.psoData = PSOData{  
                false,              // enable depth 
                false,              // blend add
                ResourceData{},     // depth stencil data
                (int)mClientWidth,
                (int)mClientHeight,
                false,              // conservative
                true,               // compute pass
            };
            data.shaders = {
                ShaderData{std::string("../assets/shaders/Voxel/MipmapVoxel.hlsl"), ShaderEnum::CS},
            };

            MipLevel mipL = MipLevel{(unsigned int)round};
            Renderer::GDevice->SetShaderConstant("MipConstant", &mipL);
        },
        [=](){
            Renderer::GContext->GetContext()->Dispatch(length*length*length,1,1);
            // barrier
            Renderer::GContext->GetContext()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(nullptr));
        });

        length /= 2;
    }
}

void Graphics::DrawSkyBox()
{
    skyBoxMgr->PrePass();
    auto rtHandle = Renderer::ResManager->GetCPU("ColorBuffer", ResourceEnum::View::RTView);
    auto dsvHandle = Renderer::ResManager->GetCPU("GBufferDepth", ResourceEnum::View::DSView);
    Context::GetContext()->OMSetRenderTargets(1, &D3D12_CPU_DESCRIPTOR_HANDLE(rtHandle), false, &dsvHandle);
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

void Graphics::GeneratePrimitive()
{
    vector<Vertex> vs;
    vector<unsigned int> ids;

    float ver[] = {
        -1.0f,-1.0f,-1.0f, // triangle 1 : begin
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, // triangle 1 : end

        1.0f, 1.0f,-1.0f, // triangle 2 : begin
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f, // triangle 2 : end

        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,

        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,

        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,

        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,

        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,

        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,

        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,

        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,

        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,

        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f
    };
    for(int i = 0; i < 36; i++){
        vs.push_back(Vertex(glm::vec3(ver[i*3],ver[i*3+1],ver[i*3+2])));
    }
    for(int i = 0; i < 36; i++) ids.push_back(i);

    cubeMesh = std::make_shared<DMesh>();
    cubeMesh->BuildVertexAndIndexBuffer(md3dDevice.Get(), mCommandList.Get(), vs, ids);

    ids.clear();
    voxelMesh = std::make_shared<DMesh>();
    for(int i = 0; i < 36; i++) ids.push_back(i);
    voxelMesh = std::make_shared<DMesh>();
    voxelMesh->BuildVertexAndIndexBuffer(md3dDevice.Get(), mCommandList.Get(), vs, ids);
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

void Graphics::Exit()
{
    FlushCommandQueue();
    DEngine::instance->Exit();
}

