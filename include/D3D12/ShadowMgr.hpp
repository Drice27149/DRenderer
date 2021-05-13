#pragma once

#include "PreZMgr.hpp"

// @TODO: pcf + taa
class ShadowMgr: public PreZMgr {
public:
    // trick
    ShadowMgr(ID3D12Device* device, ID3D12GraphicsCommandList*  commandList, int width, int height):
        PreZMgr(device, commandList, width, height){}
    void PrePass() override;
};

