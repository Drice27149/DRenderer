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
public:
    // @TODO: 场景/物体管理
    std::shared_ptr<DMesh> objMesh;
public:
    unsigned int shadow;
    unsigned int normal;
    unsigned int baseColor;
    unsigned int metallicRoughness;
    unsigned int emissive;
public:
    unsigned mapping[aiTextureType_UNKNOWN + 1];
};
