#include "DEngine.hpp"

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








