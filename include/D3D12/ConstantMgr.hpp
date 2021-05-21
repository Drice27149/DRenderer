#pragma once

#include "DEngine.hpp"
#include "FrameResource.h"
#include "Struct.hpp"

class ConstantMgr {
public:
    ConstantMgr(ID3D12Device* device, ID3D12Fence* fence, unsigned int frameCnt, unsigned int passCnt, unsigned int objCnt);
    void Update();
    void UpdatePassConstants();
    void UpdateObjConstants();
    void UpdateStaticConstants();

    D3D12_GPU_VIRTUAL_ADDRESS GetPassConstant(unsigned long long offset);
    D3D12_GPU_VIRTUAL_ADDRESS GetObjectConstant(unsigned long long offset);

    D3D12_GPU_VIRTUAL_ADDRESS GetShadowPassConstant();
    D3D12_GPU_VIRTUAL_ADDRESS GetCameraPassConstant();
    D3D12_GPU_VIRTUAL_ADDRESS GetSceneInfoConstant(){ return sceneInfoGpu->Resource()->GetGPUVirtualAddress(); }

    std::shared_ptr<SceneInfo> GetSceneInfo() { return sceneInfo; }

public:
    ID3D12Device* device;
    ID3D12Fence* fence;
    unsigned int curFrame;
    unsigned int frameCnt;
    unsigned int passCnt;
    unsigned int objCnt;
    unsigned int jitterID;
    unsigned int jitterCnt;
    std::vector<std::unique_ptr<FrameResource>> frameResources;
public:
    // tempory, scene info is placed there
    std::shared_ptr<SceneInfo> sceneInfo = nullptr;
    // static constants buffer
    std::unique_ptr<UploadBuffer<ClusterInfo>> clusterInfo = nullptr;
    std::unique_ptr<UploadBuffer<SceneInfo>> sceneInfoGpu = nullptr;
    unsigned int viewPortWidth;
    unsigned int viewPortHeight;
};