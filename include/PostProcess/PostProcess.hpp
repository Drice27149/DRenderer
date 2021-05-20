#pragma once

#include "PassMgr.hpp"
#include "Resource.hpp"

class PostProcess: public PassMgr {
public:
    void BuildPSO() override;
    void BuildRootSig() override;
    void Pass() override;
public:
    std::vector<CD3DX12_GPU_DESCRIPTOR_HANDLE> inputs;
};

