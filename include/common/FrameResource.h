#pragma once

#include <glm/glm.hpp>
#include "d3dUtil.h"
#include "MathHelper.h"
#include "UploadBuffer.h"

struct PassUniform
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 SMView;
    glm::mat4 SMProj;
};

struct ObjectUniform
{
    glm::mat4 model;
    unsigned int id;
};

struct FrameResource
{
public:
    FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource();

    // We cannot reset the allocator until the GPU is done processing the commands.
    // So each frame needs their own allocator.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

    std::unique_ptr<UploadBuffer<PassUniform>> PassCB = nullptr;
    std::unique_ptr<UploadBuffer<ObjectUniform>> ObjectCB = nullptr;

    // Fence value to mark commands up to this fence point.  This lets us
    // check if these frame resources are still in use by the GPU.
    UINT64 Fence = 0;
};