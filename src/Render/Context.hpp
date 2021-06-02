#pragma once

#include "d3d12.h"
#include "d3dUtil.h"

using namespace Microsoft::WRL;

class Context {
public:
    static void SetContext(ComPtr<ID3D12GraphicsCommandList> cmdList);
    static ID3D12GraphicsCommandList* GetContext();

// helper function
public:
    void Barrier();

private:
    static ComPtr<ID3D12GraphicsCommandList> GCmdList;
};