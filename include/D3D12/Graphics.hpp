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

    void LoadAssets();
    void CreateTextureFromImage(string fn, ComPtr<ID3D12Resource>& m_texture, ComPtr<ID3D12Resource>& textureUploadHeap);

    void BuildDescriptorHeaps();
    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    void BuildBoxGeometry(); // TOOD: delete

    void BuildPSO();

    // New
    void DrawSkyBox();
    void DrawShadowMap();
    void DrawObjects(DrawType drawType);
    void DrawLines();

    void PreZPass();
    void PrepareCluster();
    void ExecuteComputeShader();

private:
    
    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;

    ComPtr<ID3DBlob> mvsByteCode = nullptr;
    ComPtr<ID3DBlob> mpsByteCode = nullptr;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

    ComPtr<ID3D12PipelineState> mPSO = nullptr;

    TTexture CubeTex;

    int CurrentFrame = 0;
    int PassCount = 2;
    int TextureCount = 8;
    int ClusterX = 16;
    int ClusterY = 8;
    int ClusterZ = 4;

//  -- begin of the new journey
    std::unique_ptr<DMesh> objMesh = nullptr;
    std::unique_ptr<DMesh> skyMesh = nullptr;

public:
    // pass mgr, 各种 pass 管理类
    std::shared_ptr<ConstantMgr> constantMgr = nullptr;
    std::unique_ptr<PreZMgr> preZMgr = nullptr;
    std::unique_ptr<ShadowMgr> shadowMgr = nullptr;
    std::unique_ptr<HeapMgr> heapMgr = nullptr;
    std::unique_ptr<SkyBoxMgr> skyBoxMgr = nullptr;
    std::unique_ptr<ClusterMgr> clusterMgr = nullptr;
    std::unique_ptr<LightCullMgr> lightCullMgr = nullptr;
    std::unique_ptr<DebugVisMgr> debugVisMgr = nullptr;
public:
    // new, for decouple
    void InitPassMgrs();
};

