#pragma once

#include "d3dApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "FrameResource.h"
#include "Resource.hpp"
#include "DMesh.hpp"
#include "Object.hpp"
#include "ConstantMgr.hpp"
#include "PreZMgr.hpp"
#include "ShadowMgr.hpp"
#include "HeapMgr.hpp"
#include "SkyBoxMgr.hpp"
#include "ClusterMgr.hpp"
#include "LightCullMgr.hpp"
#include "DebugVisMgr.hpp"
#include "TextureMgr.hpp"
#include "PBRMgr.hpp"
#include "GUIMgr.hpp"
#include "AAMgr.hpp"
#include "TemporalAA.hpp"
#include "ToneMapping.hpp"
#include "Bloom.hpp"
#include "PrefilterIBL.hpp"
#include "FrameGraph.hpp"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

class Renderer;

class Graphics : public D3DApp
{
public:
	Graphics(HINSTANCE hInstance);
    Graphics(const Graphics& rhs) = delete;
    Graphics& operator=(const Graphics& rhs) = delete;
	~Graphics();

	virtual bool Initialize()override;

private:
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;
    virtual void Exit() override;

    virtual void OnLMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnLMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnRMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnRMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;
    virtual void OnMouseZoom(WPARAM state)override;

    void BuildDescriptorHeaps();
    // 加载模型贴图
    void UploadTextures();
    // 加载模型
    void UploadMeshes(); 
    // new, for decouple
    void InitPassMgrs();
    // New, for ibl now 
    void PrecomputeResource();

    void DrawSkyBox();
    void DrawShadowMap();
    void DrawObjects(DrawType drawType);
    void DrawLines();
    void DrawOpaque();
    void DrawGUI();

    void PreZPass();
    void PrepareCluster();
    void ExecuteComputeShader();

    void UpdateAndAsync();

    void CreatePersistentResource();

    void AddGBufferMainPass();
    void AddLightPass();
    void AddTAAPass();
    void AddPostProcessPass();
    void AddCopyPass(std::string from, std::string to);
    void AddShadowPass();

    void VoxelizeScene(unsigned int x, unsigned int y, unsigned z);
    
private:
    int CurrentFrame = 0;
    int acFrame = 0;
    int PassCount = 2;
    int ClusterX = 16;
    int ClusterY = 8;
    int ClusterZ = 4;

//  -- begin of the new journey
    std::shared_ptr<DMesh> objMesh = nullptr;

public:
    static std::shared_ptr<ConstantMgr> constantMgr;
    static std::shared_ptr<HeapMgr> heapMgr;
    static std::shared_ptr<TextureMgr> textureMgr;
    // pass 管理
    static std::shared_ptr<PreZMgr> preZMgr;
    static std::shared_ptr<ShadowMgr> shadowMgr;
    static std::shared_ptr<SkyBoxMgr> skyBoxMgr;
    static std::shared_ptr<ClusterMgr> clusterMgr;
    static std::shared_ptr<LightCullMgr> lightCullMgr;
    static std::shared_ptr<DebugVisMgr> debugVisMgr;
    static std::shared_ptr<PBRMgr> pbrMgr;
    static std::shared_ptr<GUIMgr> guiMgr;
    static std::shared_ptr<AAMgr> aaMgr;
    static std::shared_ptr<TemporalAA> temporalAA;
    static std::shared_ptr<ToneMapping> toneMapping;
    static std::shared_ptr<Bloom> bloom;
    // in fact, this is pass too, but only excute once
    static std::shared_ptr<PrefilterIBL> prefilterIBL;

    bool firstFrame = true;

    Renderer* renderer = nullptr;

public:
    static ID3D12Device* GDevice;
    static ID3D12GraphicsCommandList* GCmdList;
    static unsigned int viewPortWidth;
    static unsigned int viewPortHeight;

private:
    // use to clear voxel grid
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuVoxel; 
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuVoxel;
};

