#include "PassMgr.hpp"

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
    
};
