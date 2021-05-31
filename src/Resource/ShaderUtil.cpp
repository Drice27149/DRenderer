#include <iostream>

#include "ShaderUtil.hpp"
#include "RenderStruct.hpp"

const wchar_t* GetWString(std::string& s){
    std::wstring wstringFn = std::wstring(s.begin(), s.end());
	const wchar_t* wcharFn = wstringFn.c_str();
    return wcharFn;
}

ComPtr<ID3DBlob> ShaderUtil::GetShader(ShaderData data)
{
    if(data.shader != nullptr)
        return data.shader;
    if(shaders.count(data.name))
        return shaders[data.name];
    if(data.type == ShaderType::CS)
        shaders[data.name] = d3dUtil::CompileShader(GetWString(data.name), nullptr, "CS", "cs_5_1");
    else if(data.type == ShaderType::GS)
        shaders[data.name] = d3dUtil::CompileShader(GetWString(data.name), nullptr, "GS", "gs_5_1");
    else if(data.type == ShaderType::PS)
        shaders[data.name] = d3dUtil::CompileShader(GetWString(data.name), nullptr, "PS", "ps_5_1");
    else
        shaders[data.name] = d3dUtil::CompileShader(GetWString(data.name), nullptr, "VS", "vs_5_1");
    return shaders[data.name];
}