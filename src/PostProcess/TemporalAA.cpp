#include "TemporalAA.hpp"
#include "Graphics.hpp"

void TemporalAA::Init()
{
    CompileShaders();
    CreateResources();

    BuildRootSig();
    BuildPSO();
}

void TemporalAA::CreateResources()
{
    // @TODO: resource manager, after post process done and color grading with filament done
}

void TemporalAA::CompileShaders()
{
	// vs = d3dUtil::CompileShader(L"..\\assets\\shaders\\debug\\cluster.hlsl", nullptr, "VS", "vs_5_1");
    // gs = d3dUtil::CompileShader(L"..\\assets\\shaders\\debug\\cluster.hlsl", nullptr, "GS", "gs_5_1");
	// ps = d3dUtil::CompileShader(L"..\\assets\\shaders\\debug\\cluster.hlsl", nullptr, "PS", "ps_5_1");
    vs = d3dUtil::CompileShader(L"..\\assets\\shaders\\AA\\taa.hlsl", nullptr, "VS", "vs_5_1");
    ps = d3dUtil::CompileShader(L"..\\assets\\shaders\\AA\\taa.hlsl", nullptr, "PS", "ps_5_1");
}

void TemporalAA::PrePass()
{   
    Graphics::GCmdList->SetPipelineState(pso.Get());
    Graphics::GCmdList->SetGraphicsRootSignature(rootSig.Get());
    auto lastFrame = Graphics::aaMgr->GetLastRenderTarget();
    auto nowFrame = Graphics::aaMgr->GetCurRTSRV();
    Graphics::GCmdList->SetGraphicsRootConstantBufferView(0, Graphics::constantMgr->GetSceneInfoConstant());
    Graphics::GCmdList->SetGraphicsRootDescriptorTable(1, lastFrame);
    Graphics::GCmdList->SetGraphicsRootDescriptorTable(2, nowFrame);
}



void TemporalAA::PostPass()
{

}

