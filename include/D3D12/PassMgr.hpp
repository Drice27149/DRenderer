#pragma once

#include "d3dUtil.h"
using Microsoft::WRL::ComPtr;

class PassMgr {
public:

public:
    ComPtr<ID3DBlob> vs;
    ComPtr<ID3DBlob> gs;
    ComPtr<ID3DBlob> ps;
    // 流水线对象
    ComPtr<ID3D12PipelineState> pso;
    // 根签名
    ComPtr<ID3D12RootSignature> rootSig;
};