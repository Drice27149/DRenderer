#include "PassMgr.hpp"
#include "Resource.hpp"
#include "HeapMgr.hpp"
#include "UploadBuffer.h"

// @TODO: change this to resource manager, after postprocessing done

struct AAPassInfo {
    unsigned int ssRate;
};

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
    CD3DX12_CPU_DESCRIPTOR_HANDLE inSrvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE inSrvGpu;

    int frame = 0;
    bool firstFrame = true;
    std::shared_ptr<Resource> renderTarget[3] = {nullptr, nullptr, nullptr};
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtSrvCpu[3];
    CD3DX12_GPU_DESCRIPTOR_HANDLE rtSrvGpu[3];
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtRtvCpu[3];
    CD3DX12_GPU_DESCRIPTOR_HANDLE rtRtvGpu[3];
    // note: one rtv , srv, dsv
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetCurRTSRV(){ return rtSrvGpu[2]; }
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetCurRTRTV(){ return rtRtvCpu[2]; }
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetNextRenderTarget(){ return rtRtvCpu[frame]; }
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetTAAResult(){ return rtSrvGpu[frame]; }
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetLastRenderTarget(){ 
        if(firstFrame){
            firstFrame = false;
            return rtSrvGpu[2];
        }
        return rtSrvGpu[!frame]; 
    }
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthBuffer(){ return dsvCpu; }
    
    // ‰∏≠ËΩ¨ËØ??->ÂÜ??
    void BeginFrame();
    // ‰∏≠ËΩ¨ÂÜ??->ËØ??, uËØ??->ÂÜ??, vÂÜ??->ËØ??, Âº¢„ÂßãÂêéÂ§ÑÁêÜ, frame++
    void StartTAA();
    void EndTAA();
public:
    std::shared_ptr<HeapMgr> heapMgr;   
    std::shared_ptr<UploadBuffer<AAPassInfo>> aaPassInfo;
private:
    ComPtr<ID3DBlob> cs = nullptr;
};
