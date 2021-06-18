#include <cassert>
#include "TextureMgr.hpp"
#include "DEngine.hpp"
#include "Parallel.hpp"

void TextureMgr::Init()
{
    // for(Object* obj: DEngine::gobjs){
    //     for (int it = aiTextureType_NONE; it <= aiTextureType_UNKNOWN; it++) {
    //         if (obj->mask & (1 << it)) {
    //             // @TODO: async
    //             std::string fn = obj->texns[it];
    //             if(!set.count(fn)){
    //                 // std::shared_ptr<Resource> texture = std::make_shared<Resource>(device, commandList);
    //                 // texture->BuildImageTexture(fn);
    //                 // // save it
    //                 // textures.push_back(texture);
    //                 CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu;
    //                 CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu;
    //                 heapMgr->GetNewSRV(srvCpu, srvGpu);
    //                 textureJobs.emplace_back(TextureJob(fn, srvCpu, srvGpu));
    //                 set[fn] = 1;
    //                 // handles[fn] = srvGpu;
    //             }
    //         }
    //     }
    // }

    // // @TODO: commandList 并行
    // auto WorkFunc = [this](int64_t id)->void{
    //     auto& job = textureJobs[id];
    //     job.texture = std::make_shared<Resource>(device, commandList);
    //     job.texture->srvCpu = job.srvCpu;
    //     job.texture->srvGpu = job.srvGpu;
    //     job.texture->BuildTextureResource(job.name);
    // };

    // // TODO: thread safe
    // parallelInit();
    // parallelFor1D(WorkFunc, textureJobs.size(), textureJobs.size());
    // parallelCleanUp();

    // for(auto& job: textureJobs){
    //     handles[job.name] = job.texture->srvGpu;
    // }
}

CD3DX12_GPU_DESCRIPTOR_HANDLE TextureMgr::GetGPUHandle(std::string name)
{
    if(!handles.count(name)){
        unsigned int InvalidTextureName = 1;
        assert(InvalidTextureName != 1);
    }

    return handles[name];
}
