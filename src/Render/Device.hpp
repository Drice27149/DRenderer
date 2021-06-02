#pragma once

#include <map>
#include "d3d12.h"
#include "d3dUtil.h"

struct ShaderData;
struct RStData;
struct PSOData;
struct PassData;
class RenderPass;
class ResourceManager;

using namespace Microsoft::WRL;

class Device {
public:
    // set/get device
    static void SetDevice(ComPtr<ID3D12Device> device);
    static ID3D12Device* GetDevice();

public:
    template<typename T>
    void SetShaderConstant(std::string name, T* data);
    void SetUpRenderPass(RenderPass& renderPass, const PassData& data);
    void ExecuteRenderPass(RenderPass& renderPass, const PassData& data);

// internal function for convinient implementation
private:
    // compile/get shader
    ID3DBlob* GetShader(ShaderData data);
    // set up stage
    ID3D12RootSignature* CreateRSt(const PassData& data);
    
private:
    // the device
    static ComPtr<ID3D12Device> GDevice;

    // resource management
    std::map<std::string, ComPtr<ID3DBlob>> shaders;
    std::vector<ComPtr<ID3D12RootSignature>> rsts;
    std::vector<ComPtr<ID3D12PipelineState>> psos;

    ResourceManager* resMgr;
};