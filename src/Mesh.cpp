#include "Mesh.hpp"
#include "GraphicAPI.hpp"

Mesh::Mesh(vector<Vertex>vs, vector<unsigned int> ids)
{
    this->vs = vs;
    this->ids = ids;
    GraphicAPI::LoadMeshToGPU(*this, this->gd);
}


