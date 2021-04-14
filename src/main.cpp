#include "Global.hpp"
#include "GraphicAPI.hpp"
#include "AssimpLoader.hpp"
#include "Mesh.hpp"
#include "Object.hpp"
#include "DEngine.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

DEngine* DEngine::instance = nullptr;

GLFWwindow* DEngine::window = nullptr;

int main()
{
    // 必须进行设置, 配置好gl运行环境, 否则运行gl函数会崩溃
    DEngine::window = GraphicAPI::InitOpenGL(800, 600);
    assert(DEngine::window != nullptr);
    // 启动引擎
    DEngine::Launch();
    assert(DEngine::instance != nullptr);

    printf("Engine launched\n");

    // 启动后才能绑定输入事件
    GraphicAPI::BindInputEvent();

    printf("Init GL window done\n");

    string cyborg  = "../assets/hulkbuster/scene.gltf";
    string car = "../assets/fallout_car_2/scene.gltf";
    string spot = "../assets/spot/spot_triangulated_good.obj";
    string backpack = "../assets/backpack/backpack.obj";

    AssimpLoader* ld = new AssimpLoader();
    Object* obj = ld->LoadFile(cyborg);
    mat4 rotTrans = glm::rotate(glm::mat4(1.0), (float)(90.0*0.5/PI), vec3(-1.0, 0.0, 0.0));
    obj->Transform(rotTrans);
    rotTrans = glm::rotate(glm::mat4(1.0), (float)(180.0*0.5/PI), vec3(0.0, -1.0, 0.0));
    obj->Transform(rotTrans);

    assert(obj);

    DEngine::GetSceneMgr().AddObject(obj);

    printf("Shaders compiled\n");

    // TODO: 判断终止
    // while(!glfwWindowShouldClose(window)){

    while(true){
        DEngine::Tick();
    }

    return 0;
}