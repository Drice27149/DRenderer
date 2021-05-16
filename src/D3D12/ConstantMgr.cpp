#include "ConstantMgr.hpp"

ConstantMgr::ConstantMgr(ID3D12Device* device, ID3D12Fence* fence, unsigned int frameCnt, unsigned int passCnt, unsigned int objCnt):
    device(device),
    fence(fence),
    frameCnt(frameCnt),
    passCnt(passCnt),
    objCnt(objCnt)
{
    curFrame = 0;

    for(int i = 0; i < passCnt; i++){
        auto newFrameRes = std::make_unique<FrameResource>(device, passCnt, objCnt);
        frameResources.push_back(std::move(newFrameRes));
    }
}

void ConstantMgr::Update()
{
    curFrame = (curFrame + 1) % frameCnt;
    auto curResource = frameResources[curFrame].get();

    if(curResource->Fence != 0 && fence->GetCompletedValue() < curResource->Fence){
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(fence->SetEventOnCompletion(curResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    UpdateObjConstants();
    UpdatePassConstants();
    UpdateStaticConstants();
}

void ConstantMgr::UpdatePassConstants()
{
    // shadow map pass constant
    glm::mat4 tempLight = glm::lookAt(vec3(-8.0, 8.0, 0.0), vec3(0.0,0.0,0.0), vec3(0.0,1.0,0.0));
    auto passCB = frameResources[curFrame]->PassCB.get();
    PassUniform temp;
    temp.view = glm::transpose(tempLight); // glm::transpose(DEngine::GetCamMgr().GetViewTransform());
    temp.proj = glm::transpose(DEngine::GetCamMgr().GetProjectionTransform());
    passCB->CopyData(0, temp);
    // camera pass constant
    temp.view = glm::transpose(DEngine::GetCamMgr().GetViewTransform());
    temp.proj = glm::transpose(DEngine::GetCamMgr().GetProjectionTransform());
    temp.SMView = glm::transpose(tempLight);
    temp.SMProj = glm::transpose(DEngine::GetCamMgr().GetProjectionTransform());
    temp.CamPos = DEngine::GetCamMgr().GetViewPos();
    passCB->CopyData(1, temp);
}

void ConstantMgr::UpdateObjConstants()
{
    auto objCB = frameResources[curFrame]->ObjectCB.get();
    auto objAddr = frameResources[curFrame]->ObjectCB->Resource()->GetGPUVirtualAddress();
    unsigned long long objByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectUniform));

    int index = 0;
    for(Object* obj: DEngine::gobjs){
        ObjectUniform temp;
        temp.model = glm::transpose(obj->GetModelTransform());
        temp.id = obj->id;
        temp.mask = obj->mask;
        objCB->CopyData(index, temp);
        index++;

        objAddr += objByteSize;
    }
}

void ConstantMgr::UpdateStaticConstants()
{
    if(clusterInfo == nullptr)
        clusterInfo = std::make_unique<UploadBuffer<ClusterInfo>>(device, 1, true);

    ClusterInfo temp;
    temp.clusterX = 16;
    temp.clusterY = 8;
    temp.clusterZ = 4;
    temp.cNear = 1.0;
    temp.cFar = 20.0;

    clusterInfo->CopyData(0, temp);
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantMgr::GetPassConstant(unsigned long long offset)
{
    auto passCB = frameResources[curFrame]->PassCB.get();
    auto passAddr = passCB->Resource()->GetGPUVirtualAddress();
    unsigned long long passByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassUniform));
    return passAddr + offset * passByteSize;
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantMgr::GetObjectConstant(unsigned long long offset)
{
    auto objCB = frameResources[curFrame]->ObjectCB.get();
    auto objAddr = frameResources[curFrame]->ObjectCB->Resource()->GetGPUVirtualAddress();
    unsigned long long objByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectUniform));
    return objAddr + offset * objByteSize;
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantMgr::GetShadowPassConstant()
{
    return GetPassConstant(0);
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantMgr::GetCameraPassConstant()
{
    return GetPassConstant(1);
}

    