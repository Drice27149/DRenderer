#include "ResourceManager.hpp"
#include "RenderStruct.hpp"
#include "Device.hpp"
#include "ResourceFatory.hpp"
#include "ViewFatory.hpp"
#include "Util.hpp"
#include "Graphics.hpp"

void ResourceManager::RegisterResource(std::string name, ComPtr<ID3D12Resource>&res)
{
    resources[name] = res;
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

ID3D12Resource* ResourceManager::GetResource(std::string name)
{
    return resources[name].Get();
}

CD3DX12_CPU_DESCRIPTOR_HANDLE ResourceManager::GetCPU(std::string name, ResourceEnum::View view)
{
    return views[name].cpu[view];
}

CD3DX12_GPU_DESCRIPTOR_HANDLE ResourceManager::GetGPU(std::string name, ResourceEnum::View view)
{
    return views[name].gpu[view];
}

void ResourceManager::ForwardFrame()
{
    frame++;
    while(bufferItems.size()!=0 && bufferItems.front().frameID<frame)
        bufferItems.pop();
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
    RegisterResource(name, res);

    // append views
    for(int i = 0; i < ResourceEnum::View::UKnownView; i++){
        if(usage&(1<<i)){
            CD3DX12_CPU_DESCRIPTOR_HANDLE cpu;
            CD3DX12_GPU_DESCRIPTOR_HANDLE gpu;
            if(i == ResourceEnum::View::SRView){
                Graphics::heapMgr->GetNewSRV(cpu, gpu);
                ViewFatory::AppendTexture2DSRV(res, dxFormat, cpu);
                RegisterHandle(name, cpu, gpu, ResourceEnum::View::SRView);
            }
            if(i == ResourceEnum::View::RTView){
                Graphics::heapMgr->GetNewRTV(cpu, gpu);
                ViewFatory::AppendRTV(res, dxFormat, cpu);
                RegisterHandle(name, cpu, gpu, ResourceEnum::View::RTView);
            }
        }
    }
}

void ResourceManager::CreateViews(ComPtr<ID3D12Resource>& res, std::string name, unsigned int usage)
{
    for(int i = 0; i < ResourceEnum::View::UKnownView; i++){
        if(usage&(1<<i)){
            if(usage == ResourceEnum::View::SRView){

            }
            if(usage == ResourceEnum::View::RTView){

            }
        }
    }
}
