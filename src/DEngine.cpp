#include "DEngine.hpp"

DEngine* DEngine::instance = nullptr;

GLFWwindow* DEngine::window = nullptr;

DEngine::DEngine()
{

}

void DEngine::Launch()
{
    if(instance == nullptr)
    {
        instance = new DEngine();
    }
}

void DEngine::Tick()
{
    instance->GetRenderMgr().Render();
}








