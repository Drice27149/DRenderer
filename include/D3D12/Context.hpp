#pragma once

#include "d3dUtil.h"

// for global variable
class Context {
public:
    Microsoft::WRL::ComPtr<ID3D12Device> device;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>  commandList;
};