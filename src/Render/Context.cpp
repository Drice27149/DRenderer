#include "Context.hpp"

ComPtr<ID3D12GraphicsCommandList> Context::GCmdList = nullptr;

void Context::SetContext(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
    GCmdList = cmdList;
}

ID3D12GraphicsCommandList* Context::GetContext()
{
    return GCmdList.Get();
}