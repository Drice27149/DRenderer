
#pragma once

#include <iostream>
#include <vector>
#include "RenderPass.hpp"

class Device;
struct PassData;
struct PSOData;
struct RStData;

class FrameGraph {
public:
    template<typename Setup, typename Execute>
    void AddPass(std::string name, Setup setup, Execute execute);
private:
    Device* device;
    std::vector<RenderPass> passes;
};