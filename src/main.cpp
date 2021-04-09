#include "Global.hpp"
#include "GraphicAPI.hpp"
#include "AssimpLoader.hpp"
#include "Mesh.hpp"

int main()
{
    AssimpLoader ld;
    string fn = "../assets/backpack/backpack.obj";
    ld.LoadFile(fn);
    Mesh mesh(ld.vs, ld.ids);
    printf("load and create mesh succeed\n");

    return 0;
}