#include "PassMgr.hpp"
#include "Resource.hpp"

class PreZMgr: public PassMgr {
public:
    PreZMgr(ID3D12Device* device, ID3D12GraphicsCommandList*  commandList, int width, int height);
    void Init() override;
    void BuildPSO() override;
    void BuildRootSig() override;
    void CompileShaders() override;
    void CreateResources() override;

    void Pass() override;
public:
    int width;
    int height;
    std::shared_ptr<Resource> depthMap;
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu;
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvCpu;
};