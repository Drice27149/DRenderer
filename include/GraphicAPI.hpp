#pragma once
#include "Global.hpp"

class Mesh;
class Shader;
class Texture;

namespace GraphicAPI {

void LoadMesh(Mesh& mesh);

void LoadImageTexture(Texture& tex, string fn, bool vflip = false);

// 测试函数
void Temp_DrawMesh(Mesh& mesh, Shader& sh,  GLFWwindow* window); 

}