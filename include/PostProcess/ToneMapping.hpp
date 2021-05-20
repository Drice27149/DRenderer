#pragma once

#include "PostProcess.hpp"

class ToneMapping: public PostProcess {
public:
    void Init() override;
    void CreateResources() override;
    void CompileShaders() override;

    void PrePass() override;
    void PostPass() override;
};