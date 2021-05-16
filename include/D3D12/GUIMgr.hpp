#include "d3dUtil.h"
#include "HeapMgr.hpp"

class GUIMgr {
public:
    GUIMgr(ID3D12Device* device, ID3D12GraphicsCommandList*  commandList): device(device), commandList(commandList){}
    void Init();
    void Update();
    void Draw();
public:
    ID3D12Device* device;
    ID3D12GraphicsCommandList*  commandList;
public:
    HWND mhMainWnd;
    std::shared_ptr<HeapMgr> heapMgr;
};