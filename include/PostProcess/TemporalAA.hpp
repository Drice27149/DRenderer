#pragma once

#include "PassMgr.hpp"
#include "PostProcess.hpp"

class TemporalAA: public PostProcess {
public:
    void Init() override;
    void CreateResources() override;
    void CompileShaders() override;

    void PrePass() override;
    void PostPass() override;
public:
    unsigned int width;
    unsigned int height;
};