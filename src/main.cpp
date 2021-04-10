#include "Global.hpp"
#include "GraphicAPI.hpp"
#include "AssimpLoader.hpp"
#include "Mesh.hpp"
#include "Object.hpp"
#include "DEngine.hpp"

DEngine* DEngine::instance = nullptr;

int main()
{
    // 先启动引擎, 因为 opengl 的鼠标回调会调用输入管理器
    DEngine::Launch();
    assert(DEngine::instance != nullptr);
    printf("Engine launched\n");
    // 必须进行设置, 配置好gl运行环境, 否则运行gl函数会崩溃
    GLFWwindow* window = GraphicAPI::InitOpenGL(800, 600);
    assert(window != nullptr);
    printf("Init GL window done\n");

    string cyborg  = "../assets/cyborg-ray-fisher/source/Cyborg_HeroPose.fbx";
    string car = "../assets/fallout_car_2/scene.gltf";
    string spot = "../assets/spot/spot_triangulated_good.obj";
    string backpack = "../assets/backpack/backpack.obj";

    AssimpLoader* ld = new AssimpLoader();
    ld->LoadFile(car);

    Mesh* mesh = new Mesh(ld->vs, ld->ids, ld->texs, ld->mask);

    printf("Mesh loaded\n");

    string vs_s = "../shaders/tvs.glsl";
    string fs_s = "../shaders/tfs.glsl";

    Shader sh(vs_s, fs_s);

    GraphicAPI::Temp_DrawMesh(*mesh, sh, window);

    

    return 0;
}