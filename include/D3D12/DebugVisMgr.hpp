#include "PassMgr.hpp"
#include "ConstantMgr.hpp"

class DebugVisMgr: public PassMgr {
public:
    DebugVisMgr(ID3D12Device* device, ID3D12GraphicsCommandList*  commandList): PassMgr(device, commandList){}
    void Init() override;
    void BuildPSO() override;
    void BuildRootSig() override;
    void CompileShaders() override;
    void CreateResources() override;

    void Pass() override;
    void PrePass() override;
    void PostPass() override;
public:
    CD3DX12_GPU_DESCRIPTOR_HANDLE offsetTable;
    CD3DX12_GPU_DESCRIPTOR_HANDLE entryTable;
    CD3DX12_GPU_DESCRIPTOR_HANDLE clusterDepth;
    // hack
    std::shared_ptr<ConstantMgr> constantMgr;
public:
    // extra pass for grid
    ComPtr<ID3DBlob> exvs;
    ComPtr<ID3DBlob> exps;
    ComPtr<ID3D12PipelineState> expso;
};