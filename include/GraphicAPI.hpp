#pragma once
#include "Global.hpp"

class Mesh;

namespace GraphicAPI {

void LoadMeshToGPU(Mesh& mesh, GraphicData& gd);

// ��ʱ����, ��������֤ assimp ����ģ�ͳɹ�
void Temp_DrawMesh(Mesh& mesh); 

}