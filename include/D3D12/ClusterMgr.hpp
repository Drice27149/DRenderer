#include "PassMgr.hpp"
#include "Resource.hpp"
#include "ConstantMgr.hpp"

class ClusterMgr: public PassMgr {
public:
    ClusterMgr(ID3D12Device* device, 
        ID3D12GraphicsCommandList* commandList,
        unsigned int clusterX, 
        unsigned int clusterY, 
        unsigned int clusterZ
    );
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
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu;
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE rtvGpu;
public:
    std::shared_ptr<Resource> depthMap;
public:
    unsigned int clusterX;
    unsigned int clusterY;
    unsigned int clusterZ;
public:
    // 临时, 之后会有共享上下文
    std::shared_ptr<ConstantMgr> constantMgr;
    // 临时 debug 用, 之后始终使用正常相机
    std::unique_ptr<UploadBuffer<PassUniform>> fixCam = nullptr;
    std::unique_ptr<UploadBuffer<ClusterInfo>> clusterInfo = nullptr;
};