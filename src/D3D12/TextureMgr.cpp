#include <cassert>>
#include "TextureMgr.hpp"
#include "DEngine.hpp"

void TextureMgr::Init()
{
    for(Object* obj: DEngine::gobjs){
        for (int it = aiTextureType_NONE; it <= aiTextureType_UNKNOWN; it++) {
            if (obj->mask & (1 << it)) {
                // @TODO: async
                std::string fn = obj->texns[it];
                if(!handles.count(fn)){
                    std::shared_ptr<Resource> texture = std::make_shared<Resource>(device, commandList);
                    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu;
                    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu;
                    heapMgr->GetNewSRV(srvCpu, srvGpu);
                    texture->srvCpu = srvCpu;
                    texture->srvGpu = srvGpu;
                    texture->BuildImageTexture(fn);
                    // save it
                    textures.push_back(texture);
                    handles[fn] = srvGpu;
                }
            }
        }
    }
}

CD3DX12_GPU_DESCRIPTOR_HANDLE TextureMgr::GetGPUHandle(std::string name)
{
    if(!handles.count(name)){
        unsigned int InvalidTextureName = 1;
        assert(InvalidTextureName != 1);
    }

    return handles[name];
}
