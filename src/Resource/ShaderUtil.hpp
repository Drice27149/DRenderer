#pragma once

#include <map>
#include "d3dUtil.h"

using namespace Microsoft::WRL;

struct ShaderData;

class ShaderUtil {
public:
    ComPtr<ID3DBlob> GetShader(ShaderData data);
private:
    std::map<std::string, ComPtr<ID3DBlob>> shaders;
};