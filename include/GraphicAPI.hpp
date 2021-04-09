#pragma once
#include "Global.hpp"

class Mesh;

namespace GraphicAPI {

void LoadMeshToGPU(Mesh& mesh, GraphicData& gd);

// 临时函数, 仅用于验证 assimp 导入模型成功
void Temp_DrawMesh(Mesh& mesh); 

}