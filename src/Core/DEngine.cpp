#include "DEngine.hpp"
#include "ConfigLoader.hpp"
#include "Reflect.hpp"

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

void DEngine::Init()
{

}

void DEngine::Exit()
{
    std::vector<Reflect::Element> datas;
    for(Object* obj: gobjs){
        datas.push_back(Reflect::Element{obj->serialize()});
    }
    ConfigLoader::SaveConfig("../Save/Config.txt", datas);
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








