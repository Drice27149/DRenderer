#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include "d3dx12.h"
#include "d3dUtil.h"

using namespace Microsoft::WRL;

// just for voxel gird ...

class Mipmap {
public:
    static void CreateMipmap(unsigned int length);
    static void ProcessMipmap(unsigned int length);
public:
    static CD3DX12_CPU_DESCRIPTOR_HANDLE mipCpu[16];
    static CD3DX12_GPU_DESCRIPTOR_HANDLE mipGpu[16];
};
