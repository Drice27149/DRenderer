#pragma once

#include "PostProcess.hpp"

class ToneMapping: public PostProcess {
public:
    void Init() override;
    void CreateResources() override;
    void CompileShaders() override;

    void PrePass() override;
    void PostPass() override;
public:
    D3D12_GPU_DESCRIPTOR_HANDLE input;
};