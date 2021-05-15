#include "DEngine.hpp"

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
    // instance->GetRenderMgr().Render();
}

void DEngine::LogError(string s)
{
    freopen("log.txt", "w", stdout);

    std::cout << s <<"\n";

    fclose(stdout);
}








