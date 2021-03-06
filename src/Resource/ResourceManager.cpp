#include <cassert>

#include "ResourceManager.hpp"
#include "RenderStruct.hpp"
#include "Device.hpp"
#include "Context.hpp"
#include "ResourceFatory.hpp"
#include "ViewFatory.hpp"
#include "Util.hpp"
#include "Graphics.hpp"
#include "DEngine.hpp"

DXGI_FORMAT GetDXFormat(ResourceEnum::Format format)
{
    DXGI_FORMAT mapping[35];
    mapping[ResourceEnum::Format::R16G16_FLOAT] = DXGI_FORMAT_R16G16_FLOAT;
    mapping[ResourceEnum::Format::R32_UINT] = DXGI_FORMAT_R32_UINT;
    mapping[ResourceEnum::Format::R8G8B8A8_UNORM] = DXGI_FORMAT_R8G8B8A8_UNORM;
    mapping[ResourceEnum::Format::R32G32B32A32_UINT] = DXGI_FORMAT_R32G32B32A32_UINT;
    mapping[ResourceEnum::Format::R32G32B32A32_FLOAT] = DXGI_FORMAT_R32G32B32A32_FLOAT;

    return mapping[format];
}

void ResourceManager::RegisterResource(std::string name, ComPtr<ID3D12Resource>&res, ResourceEnum::State state)
{
    resources[name] = res.Get();
    stateTrack[name] = state;
}

void ResourceManager::RegisterResource(std::string name, ID3D12Resource* res, ResourceEnum::State state)
{
    resources[name] = res;
    stateTrack[name] = state;
}

void ResourceManager::RegisterResourceInfo(std::string name, ResourceEnum::Type type)
{
    typeTrack[name] = type;
}

ResourceEnum::State ResourceManager::GetResourceState(std::string name)
{
    assert(stateTrack.count(name));
    return stateTrack[name];
}

ResourceEnum::Type ResourceManager::GetResourceType(std::string name)
{
    assert(typeTrack.count(name));
    return typeTrack[name];
}

void ResourceManager::ResourceBarrier(std::string name, ResourceEnum::State dest)
{
    assert(stateTrack.count(name));
    if(stateTrack[name] == dest)
        return ;
    
    D3D12_RESOURCE_STATES readState;
    D3D12_RESOURCE_STATES writeState;
    auto resType = GetResourceType(name);
    if(resType == ResourceEnum::Type::DepthStencil)
        writeState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    else if(resType == ResourceEnum::Type::Texture2D)
        writeState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    else if(resType == ResourceEnum::Type::Texture3D)
        writeState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    readState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    if(dest == ResourceEnum::State::Read){
        Context::GetContext()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            GetResource(name),
            writeState,
            readState
            )
        );
    }
    else if(dest == ResourceEnum::State::Write){
        Context::GetContext()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
            GetResource(name),
            readState,
            writeState
            )
        );
    }
    else{
        assert(0);
    }
    stateTrack[name] = dest;
}

void ResourceManager::RegisterHandle(std::string name,CD3DX12_CPU_DESCRIPTOR_HANDLE handle, ResourceEnum::View view)
{
    if(!views.count(name)){
        views[name] = ResourceView();
    }
    views[name].mask |= 1<<view;
    views[name].cpu[view] = handle;
}

void ResourceManager::RegisterHandle(std::string name, CD3DX12_GPU_DESCRIPTOR_HANDLE handle, ResourceEnum::View view)
{
    if(!views.count(name)){
        views[name] = ResourceView();
    }
    views[name].mask |= 1<<view;
    views[name].gpu[view] = handle;
}

void ResourceManager::RegisterHandle(std::string name, CD3DX12_CPU_DESCRIPTOR_HANDLE chandle, CD3DX12_GPU_DESCRIPTOR_HANDLE ghandle, ResourceEnum::View view)
{
    ResourceManager::RegisterHandle(name, chandle, view);
    ResourceManager::RegisterHandle(name, ghandle, view);
}

void ResourceManager::RegisterStateTrack(std::string name, ResourceEnum::State state)
{
    stateTrack[name] = state;
}
    
void ResourceManager::RegisterTypeTrack(std::string name, ResourceEnum::Type type)
{
    typeTrack[name] = type;
}

ID3D12Resource* ResourceManager::GetResource(std::string name)
{
    assert(resources.count(name));
    return resources[name];
}

CD3DX12_CPU_DESCRIPTOR_HANDLE ResourceManager::GetCPU(std::string name, ResourceEnum::View view)
{
    assert(views.count(name));
    return views[name].cpu[view];
}

CD3DX12_GPU_DESCRIPTOR_HANDLE ResourceManager::GetGPU(std::string name, ResourceEnum::View view)
{
    assert(views.count(name));
    return views[name].gpu[view];
}

void ResourceManager::ForwardFrame()
{
    frame++;
}

void ResourceManager::ForwardResource()
{
    if(bufferItems.size()==0)
        return ;
    int head = bufferItems.front().frameID;
    while(bufferItems.size()!=0 && bufferItems.front().frameID == head){
        delete bufferItems.front().buffer;
        bufferItems.pop();
    }
}

void ResourceManager::CreateRenderTarget(std::string name, ResourceDesc desc, unsigned int usage)
{
    // create and register resource
    ComPtr<ID3D12Resource> res;
    DXGI_FORMAT dxFormat;
    if(desc.format == ResourceEnum::Format::R32G32B32A32_FLOAT)
        dxFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
    else if(desc.format == ResourceEnum::Format::R8G8B8A8_UNORM)
        dxFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    else if(desc.format == ResourceEnum::Format::R16G16_FLOAT)
        dxFormat = DXGI_FORMAT_R16G16_FLOAT;
    ResFatory::CreateRenderTarget2DResource(res, desc.width, desc.height, dxFormat);
    res->SetName(WString(name).c_str());
    RegisterResource(name, res, ResourceEnum::State::Write);
    RegisterResourceInfo(name, ResourceEnum::Type::Texture2D);

    ResourceManager::CreateViews(res, name, usage);

    // keep ref from GC
    resPools.push_back(res);
}

void ResourceManager::CreateViews(ComPtr<ID3D12Resource>& res, std::string name, unsigned int usage)
{
    for(int i = 0; i < ResourceEnum::View::UKnownView; i++){
        if(usage&(1<<i)){
            CD3DX12_CPU_DESCRIPTOR_HANDLE cpu;
            CD3DX12_GPU_DESCRIPTOR_HANDLE gpu;
            if(i == ResourceEnum::View::SRView){
                Graphics::heapMgr->GetNewSRV(cpu, gpu);
                if(res->GetDesc().Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
                    ViewFatory::AppendTexture3DSRV(res, res->GetDesc().Format, cpu);
                else
                    ViewFatory::AppendTexture2DSRV(res, res->GetDesc().Format, cpu);
                RegisterHandle(name, cpu, gpu, ResourceEnum::View::SRView);
            }
            if(i == ResourceEnum::View::RTView){
                Graphics::heapMgr->GetNewRTV(cpu, gpu);
                ViewFatory::AppendRTV(res, res->GetDesc().Format, cpu);
                RegisterHandle(name, cpu, gpu, ResourceEnum::View::RTView);
            }
            if(i == ResourceEnum::View::DSView){
                Graphics::heapMgr->GetNewDSV(cpu, gpu);
                // hack dsv format here
                auto dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
                ViewFatory::AppendDSV(res, dsvFormat, cpu);
                RegisterHandle(name, cpu, gpu, ResourceEnum::View::DSView);
            }
            if(i == ResourceEnum::View::UAView){
                Graphics::heapMgr->GetNewSRV(cpu, gpu);
                D3D12_UAV_DIMENSION viewDim;

                auto type = typeTrack[name];
                if(type == ResourceEnum::Type::Texture3D)
                    viewDim = D3D12_UAV_DIMENSION_TEXTURE3D;
                else if(type == ResourceEnum::Type::Texture2D)
                    viewDim = D3D12_UAV_DIMENSION_TEXTURE2D;
                else if(type == ResourceEnum::Type::Buffer)
                    viewDim = D3D12_UAV_DIMENSION_BUFFER;
                else 
                    viewDim = D3D12_UAV_DIMENSION_UNKNOWN;

                ViewFatory::AppendUAV(res, res->GetDesc().Format, viewDim, cpu);
                RegisterHandle(name, cpu, gpu, ResourceEnum::View::UAView);
            }
        }
    }
}

void ResourceManager::CreateImageTexture(std::string name, unsigned int usage)
{
    ComPtr<ID3D12Resource> res;
    ComPtr<ID3D12Resource> uploadBuffer;

    ResFatory::CreateImageTexture(res, uploadBuffer, name);
    res->SetName(WString(name).c_str());
    RegisterResource(name, res);
    RegisterResourceInfo(name, ResourceEnum::Type::Texture2D);
    // create views
    ResourceManager::CreateViews(res, name, usage);
    // keep ref from GC
    resPools.push_back(res);
    resPools.push_back(uploadBuffer);
}

void ResourceManager::LoadObjectTextures()
{
    for(Object* obj: DEngine::gobjs){
        for(Mesh& mesh: obj->meshes){
            for (int it = aiTextureType_NONE; it <= aiTextureType_UNKNOWN; it++) {
                if (mesh.mask & (1 << it)) {
                    std::string fn = mesh.texns[it];
                    if(!resources.count(fn)){
                        CreateImageTexture(
                            fn,
                            1<<ResourceEnum::SRView
                        );
                    }
                }
            }
        }
    }
}

void ResourceManager::CreateDepthStencil(std::string name, ResourceDesc desc, unsigned int usage)
{
    ComPtr<ID3D12Resource> res;
    DXGI_FORMAT format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    ResFatory::CreateDepthStencil(res, desc.width, desc.height, format);
    res->SetName(WString(name).c_str());
    RegisterResource(name, res, ResourceEnum::State::Write);
    RegisterResourceInfo(name, ResourceEnum::Type::DepthStencil);
    // create views
    ResourceManager::CreateViews(res, name, usage);
    // keep ref from GC
    resPools.push_back(res);
}

void ResourceManager::CreateTexture3D(std::string name, ResourceDesc desc, unsigned int depth, unsigned int usage, unsigned int mipLevel)
{
    ComPtr<ID3D12Resource> res;
    // temporal hack
    DXGI_FORMAT format = GetDXFormat(desc.format);
    ResFatory::CreateTexture3D(res, desc.width, desc.height, depth, format, mipLevel);
    res->SetName(WString(name).c_str());
    RegisterResource(name, res, desc.state);
    RegisterResourceInfo(name, desc.type);
    // create views
    ResourceManager::CreateViews(res, name, usage);
    // keep ref from GC
    resPools.push_back(res);
}
