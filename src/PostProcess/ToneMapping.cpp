#include "ToneMapping.hpp"
#include "Graphics.hpp"

void ToneMapping::Init()
{
    CompileShaders();
    CreateResources();

    BuildRootSig();
    BuildPSO();
}

void ToneMapping::CreateResources()
{
    // @TODO: resource manager, after post process done and color grading with filament done
}

void ToneMapping::CompileShaders()
{
    vs = d3dUtil::CompileShader(L"..\\assets\\shaders\\AA\\toneMapping.hlsl", nullptr, "VS", "vs_5_1");
    ps = d3dUtil::CompileShader(L"..\\assets\\shaders\\AA\\toneMapping.hlsl", nullptr, "PS", "ps_5_1");
}

void ToneMapping::PrePass()
{   
    Graphics::GCmdList->SetPipelineState(pso.Get());
    Graphics::GCmdList->SetGraphicsRootSignature(rootSig.Get());
    auto taaRes = Graphics::aaMgr->GetTAAResult();
    Graphics::GCmdList->SetGraphicsRootDescriptorTable(0, taaRes);
}

void ToneMapping::PostPass()
{

}

