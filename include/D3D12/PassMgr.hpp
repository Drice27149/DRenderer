#pragma once

#include "d3dUtil.h"
#include "Global.hpp"
using Microsoft::WRL::ComPtr;

class PassMgr {
public:
    PassMgr(ID3D12Device* device, ID3D12GraphicsCommandList*  commandList): device(device), commandList(commandList){}
public:
    virtual void Init() = 0;
    virtual void BuildRootSig() = 0;
    virtual void BuildPSO() = 0;
    virtual void CreateResources() = 0;
    virtual void CompileShaders() = 0;

    virtual void Pass() = 0;
    // @TODO: do it better
    virtual void PrePass() = 0;
    virtual void PostPass() = 0;
public:
    // 创建资源和命令提交
    ID3D12Device* device;
    ID3D12GraphicsCommandList*  commandList;
    // 各阶段着色器
    ComPtr<ID3DBlob> vs;
    ComPtr<ID3DBlob> gs;
    ComPtr<ID3DBlob> ps;
    // 流水线对象
    ComPtr<ID3D12PipelineState> pso;
    // 根签名
    ComPtr<ID3D12RootSignature> rootSig;
    // 顶点格式
    const std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = 
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, vertex), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, texCoord), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, tangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, bitangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 1, DXGI_FORMAT_R32_UINT, 0, offsetof(Vertex, x), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 2, DXGI_FORMAT_R32_UINT, 0, offsetof(Vertex, y), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        { "TEXCOORD", 3, DXGI_FORMAT_R32_UINT, 0, offsetof(Vertex, z), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
};