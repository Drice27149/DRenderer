#include "PassMgr.hpp"
#include "Resource.hpp"

class LightCullMgr: public PassMgr {
public:
    LightCullMgr(ID3D12Device* device, 
        ID3D12GraphicsCommandList*  commandList,
        unsigned int clusterX,
        unsigned int clusterY,
        unsigned int clusterZ
    );
    void Init() override;
    void BuildPSO() override;
    void BuildRootSig() override;
    void CompileShaders() override;
    void CreateResources() override;

    void Pass() override;
    void PrePass() override;
    void PostPass() override;
public:
    // compute shader
    ComPtr<ID3DBlob> cs = nullptr;
    // ืสิด
    ComPtr<ID3D12Resource> HeadTable;
    ComPtr<ID3D12Resource> NodeTable, NodeTableCounter;
    ComPtr<ID3D12Resource> LightTable;
    ComPtr<ID3D12Resource> LightUploadBuffer;

    CD3DX12_GPU_DESCRIPTOR_HANDLE HeadTableHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE NodeTableHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE LightTableHandle;

    // head: clear table, node: clear counter
    ComPtr<ID3D12Resource> headClearBuffer;
    ComPtr<ID3D12Resource> nodeClearBuffer;
    ComPtr<ID3D12Resource> headUploadBuffer;
    ComPtr<ID3D12Resource> nodeUploadBuffer;

    
public:
    unsigned int clusterX;
    unsigned int clusterY;
    unsigned int clusterZ;
};