#pragma once
#include "Global.hpp"
#include "Mesh.hpp"

namespace GraphicAPI {

void myPrint();

unsigned int LoadMeshToGPU(Mesh*);
}