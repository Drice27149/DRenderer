#include "PassMgr.hpp"
#include "Resource.hpp"
#include "HeapMgr.hpp"

class AAMgr: public PassMgr {
public:
    AAMgr(ID3D12Device* device, ID3D12GraphicsCommandList* commandList): PassMgr(device, commandList){}

    void Init() override;
    void BuildPSO() override;
    void BuildRootSig() override;
    void CompileShaders() override;
    void CreateResources() override;

    void Pass() override;
    void PrePass() override;
    void PostPass() override;
public:
    void Update(unsigned int width, unsigned int height);
public:
    unsigned int ssRate;
    unsigned int sWidth;
    unsigned int sHeight;
    unsigned int width;
    unsigned int height;
    std::shared_ptr<Resource> InRTV;    // high res for ssaa
    std::shared_ptr<Resource> depthMap; // depth map for ssaa
    std::shared_ptr<Resource> OutRTV;   // resolve color
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCpu;
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvCpu;
public:
    std::shared_ptr<HeapMgr> heapMgr;   
private:
    ComPtr<ID3DBlob> cs = nullptr;
};
