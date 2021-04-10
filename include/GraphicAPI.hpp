#pragma once
#include "Global.hpp"

class Mesh;
class Shader;

namespace GraphicAPI {

void LoadMeshToGPU(Mesh& mesh);

// 测试函数
void Temp_DrawMesh(Mesh& mesh, Shader& sh,  GLFWwindow* window); 

}