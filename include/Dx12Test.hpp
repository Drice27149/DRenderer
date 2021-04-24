#include "d3dApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "FrameResource.h"
#include "ShadowMap.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

struct ObjectConstants
{
    XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
};


class BoxApp : public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
    BoxApp(const BoxApp& rhs) = delete;
    BoxApp& operator=(const BoxApp& rhs) = delete;
	~BoxApp();

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
    void LoadTexture();
    void LoadCubeMap();
    void CreateTextureFromImage(string fn, ComPtr<ID3D12Resource>& m_texture, ComPtr<ID3D12Resource>& textureUploadHeap);

    void UpdateObjUniform();
    void UpdatePassUniform();
    void UpdateLegacy();

    void BuildDescriptorHeaps();
	void BuildConstantBuffers();
    void BuildConstantBufferView();
    void BuildShaderResourceView();
    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    void BuildBoxGeometry();
    void BuildPSO();
    void BuildFrameResources();

    void DrawSkyBox();
    void DrawShadowMap();
    void DrawObjects();

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

private:
    
    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;
    ComPtr<ID3D12DescriptorHeap> mDsvHeap = nullptr;

    std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;

	std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;

    ComPtr<ID3DBlob> mvsByteCode = nullptr;
    ComPtr<ID3DBlob> mpsByteCode = nullptr;
    ComPtr<ID3DBlob> shadowVS = nullptr;
    ComPtr<ID3DBlob> shadowPS = nullptr;
    ComPtr<ID3DBlob> skyVS = nullptr;
    ComPtr<ID3DBlob> skyPS = nullptr;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

    ComPtr<ID3D12PipelineState> mPSO = nullptr;
    ComPtr<ID3D12PipelineState> SMPSO = nullptr;
    ComPtr<ID3D12PipelineState> SkyPSO = nullptr;

    XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
    XMFLOAT4X4 mView = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj = MathHelper::Identity4x4();

    float mTheta = 1.5f*XM_PI;
    float mPhi = XM_PIDIV4;
    float mRadius = 5.0f;

    std::vector<TTexture> textures;

    TTexture MyTex;
    POINT mLastMousePos;

    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    std::unique_ptr<MeshGeometry> mMeshGeo = nullptr;
    std::vector<unsigned int> mMeshIndex;

    CD3DX12_CPU_DESCRIPTOR_HANDLE SMHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE GPUSMHandle;

    std::unique_ptr<ShadowMap> shadowMap;
    TTexture CubeTex;

    int CurrentFrame = 0;
    int PassCount = 2;
};

