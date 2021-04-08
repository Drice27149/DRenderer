#include "Global.hpp"
#include "GraphicAPI.hpp"
#include "AssimpLoader.hpp"

int main()
{
    AssimpLoader ld;
    string fn = "../assets/backpack/backpack.obj";
    ld.LoadFile(fn);
    GraphicAPI::myPrint();

    return 0;
}