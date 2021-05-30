#pragma once

#include "PassMgr.hpp"
#include "ConstantMgr.hpp"
#include "TextureMgr.hpp"
#include "ShadowMgr.hpp"
#include "LightCullMgr.hpp"
#include "DMesh.hpp"
#include "SkyBoxMgr.hpp"

class PBRMgr: public PassMgr {
public:
    PBRMgr(ID3D12Device* device, ID3D12GraphicsCommandList*  commandList);
public:
    void Init() override;
    void BuildPSO() override;
    void BuildRootSig() override;
    void CompileShaders() override;
    void CreateResources() override;

    void Pass() override;
    void PrePass() override;
    void PostPass() override;

    CD3DX12_CPU_DESCRIPTOR_HANDLE GetVelocityRTV(){ return velRtvCpu; }
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetVelocitySRV(){ return velSrvGpu; }
    ID3D12Resource* GetVelocityResource(){ return velocity.Get(); }
public:
    // @TODO: 场景/物体管理
    std::shared_ptr<DMesh> objMesh;
public:
    unsigned int shadow;
    unsigned int normal;
    unsigned int baseColor;
    unsigned int metallicRoughness;
    unsigned int emissive;
    unsigned int width;
    unsigned int height;
public:
    unsigned mapping[aiTextureType_UNKNOWN + 1];
    ComPtr<ID3D12Resource> velocity;
    CD3DX12_CPU_DESCRIPTOR_HANDLE velRtvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE velRtvGpu;
    CD3DX12_CPU_DESCRIPTOR_HANDLE velSrvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE velSrvGpu;
};
