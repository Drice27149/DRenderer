#include "GraphicAPI.hpp"

namespace GraphicAPI {

void myPrint()
{
    printf("checked");
}

unsigned int LoadMeshToGPU(Mesh* mesh)
{
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

}

}