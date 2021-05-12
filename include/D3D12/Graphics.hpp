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

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

struct TempOffset {
    unsigned int offset;
};

struct TempNode {
    unsigned int id;
    unsigned int next;
};

struct TempLight {
    unsigned int id;
    glm::vec3 pos;
    float radiance;
};

struct TempCluster {
    unsigned int clusterX;
    unsigned int clusterY;
    unsigned int clusterZ;
    float cNear;
    float cFar;
};

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
    void BuildShaderResourceView();
    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    void BuildBoxGeometry(); // TOOD: delete
    void BuildDebugCluster();
    void BuildPSO();
    void BuildFrameResources();

    // New
    void DrawSkyBox();
    void DrawShadowMap();
    void DrawObjects(DrawType drawType);
    void DrawLines();

    void InitDescriptorHeaps();
    void InitSRV();

    void PreZPass();
    void PrepareCluster();
    void InitUAV();
    void PrepareComputeShader();

    void ExecuteComputeShader();

    void ClearUAVs();

    void PrepareClusterVis();
    void BuildClusterVisPSO();

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

private:
    
    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;

    ComPtr<ID3DBlob> mvsByteCode = nullptr;
    ComPtr<ID3DBlob> mpsByteCode = nullptr;
    ComPtr<ID3DBlob> shadowVS = nullptr;
    ComPtr<ID3DBlob> shadowPS = nullptr;
    ComPtr<ID3DBlob> skyVS = nullptr;
    ComPtr<ID3DBlob> skyPS = nullptr;
    ComPtr<ID3DBlob> clusterCS = nullptr;
    ComPtr<ID3DBlob> debugCS = nullptr;
    
    ComPtr<ID3DBlob> clusterVS = nullptr;
    ComPtr<ID3DBlob> clusterGS = nullptr;
    ComPtr<ID3DBlob> clusterPS = nullptr;

    ComPtr<ID3DBlob> clusterVisVS = nullptr;
    ComPtr<ID3DBlob> clusterVisGS = nullptr;
    ComPtr<ID3DBlob> clusterVisPS = nullptr;

    ComPtr<ID3D12PipelineState> clusterVisPSO = nullptr;
    ComPtr<ID3D12RootSignature> clusterVisSignature = nullptr;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

    ComPtr<ID3D12PipelineState> mPSO = nullptr;

    XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
    XMFLOAT4X4 mView = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj = MathHelper::Identity4x4();

    float mTheta = 1.5f*XM_PI;
    float mPhi = XM_PIDIV4;
    float mRadius = 5.0f;

    std::vector<TTexture> textures;

    TTexture MyTex;
    POINT mLastMousePos;

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

    std::unique_ptr<Resource> clusterDepth;

    ComPtr<ID3D12PipelineState> clusterPSO = nullptr;
    ComPtr<ID3D12PipelineState> computePSO = nullptr;
    ComPtr<ID3D12PipelineState> debugPSO = nullptr;
    ComPtr<ID3D12Resource> HeadTable, HeadTableCounter;
    ComPtr<ID3D12Resource> NodeTable, NodeTableCounter;
    ComPtr<ID3D12Resource> LightTable;
    ComPtr<ID3D12Resource> LightUploadBuffer;

    ComPtr<ID3D12RootSignature> CSRootSignature;

    CD3DX12_GPU_DESCRIPTOR_HANDLE HeadTableHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE NodeTableHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE LightTableHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE DebugTableHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE SkyTexHandle;

    ComPtr<ID3D12Resource> debugTexture;
    ComPtr<ID3D12Resource> headClearBuffer;
    ComPtr<ID3D12Resource> nodeCounterClearBuffer;
    ComPtr<ID3D12Resource> headUploadBuffer;
    ComPtr<ID3D12Resource> nodeUploadBuffer;

    std::unique_ptr<UploadBuffer<PassUniform>> fixCamCB = nullptr;

public:
    // pass mgr, 各种 pass 管理类
    std::shared_ptr<ConstantMgr> constantMgr = nullptr;
    std::unique_ptr<PreZMgr> preZMgr = nullptr;
    std::unique_ptr<ShadowMgr> shadowMgr = nullptr;
    std::unique_ptr<HeapMgr> heapMgr = nullptr;
    std::unique_ptr<SkyBoxMgr> skyBoxMgr = nullptr;
    std::unique_ptr<ClusterMgr> clusterMgr = nullptr;
public:
    // new, for decouple
    void InitPassMgrs();
};

