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

    vec2 a(-2000, 2000);
    vec2 b(2000, 2000);
    vec2 c(2000, -2000);
    vec2 d(-2000, -2000);

    Grid* grid = new Grid(a, b, c, d, 20);

    string l_vs = "../shaders/grid/line.vs";
    string l_fs = "../shaders/grid/line.fs";
    
    Shader l_sh(l_vs, l_fs);

    glEnable(GL_DEPTH_TEST);

    // TODO: 把渲染循环放在别处
    while(!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GraphicAPI::Temp_DrawGrid(*grid, l_sh);
        GraphicAPI::Temp_DrawMesh(*mesh, sh);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}