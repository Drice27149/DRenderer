#pragma once

#include <map>
#include "d3d12.h"
#include "d3dUtil.h"
#include "ResourceManager.hpp"
#include "RenderEnum.hpp"

struct ShaderData;
struct RStData;
struct PSOData;
struct PassData;
class RenderPass;

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
    std::map<std::string, ComPtr<ID3DBlob>>& GetShaderMap(ShaderEnum::Type type);

private:
    // the device
    static ComPtr<ID3D12Device> GDevice;

    // resource management
    std::map<std::string, ComPtr<ID3DBlob>> vss;
    std::map<std::string, ComPtr<ID3DBlob>> gss;
    std::map<std::string, ComPtr<ID3DBlob>> pss;
    std::map<std::string, ComPtr<ID3DBlob>> css;
    std::vector<ComPtr<ID3D12RootSignature>> rsts;
    std::vector<ComPtr<ID3D12PipelineState>> psos;
};

template<typename T>
void Device::SetShaderConstant(std::string name, T* data)
{
    Renderer::ResManager->CommitConstantBuffer(name, data);
}