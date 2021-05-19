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

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

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
    // New
    void DrawSkyBox();
    void DrawShadowMap();
    void DrawObjects(DrawType drawType);
    void DrawLines();
    void DrawOpaque();
    void DrawGUI();

    void PreZPass();
    void PrepareCluster();
    void ExecuteComputeShader();
    
private:
    int CurrentFrame = 0;
    int PassCount = 2;
    int ClusterX = 16;
    int ClusterY = 8;
    int ClusterZ = 4;

//  -- begin of the new journey
    std::shared_ptr<DMesh> objMesh = nullptr;

public:
    // 资源管理
    std::shared_ptr<ConstantMgr> constantMgr = nullptr;
    std::shared_ptr<HeapMgr> heapMgr = nullptr;
    std::shared_ptr<TextureMgr> textureMgr = nullptr;
    // pass 管理
    std::unique_ptr<PreZMgr> preZMgr = nullptr;
    std::shared_ptr<ShadowMgr> shadowMgr = nullptr;
    std::shared_ptr<SkyBoxMgr> skyBoxMgr = nullptr;
    std::unique_ptr<ClusterMgr> clusterMgr = nullptr;
    std::shared_ptr<LightCullMgr> lightCullMgr = nullptr;
    std::unique_ptr<DebugVisMgr> debugVisMgr = nullptr;
    std::unique_ptr<PBRMgr> pbrMgr = nullptr;
    std::unique_ptr<GUIMgr> guiMgr = nullptr;
    std::shared_ptr<AAMgr> aaMgr = nullptr;
public:
};

