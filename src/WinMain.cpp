
#include "Global.hpp"
#include "Object.hpp"
#include "AssimpLoader.hpp"
#include "DEngine.hpp"
#include "Dx12Test.hpp"

Object* DEngine::gobj = nullptr;
DEngine* DEngine::instance = nullptr;
std::vector<Object*> DEngine::gobjs;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    try
    {
        DEngine::Launch();

        BoxApp theApp(hInstance);

        string fn = "../assets/fallout_car_2/scene.gltf";
        AssimpLoader* ld = new AssimpLoader();
        DEngine::gobj = ld->LoadFile(fn);

        DEngine::gobjs.clear();

        for(int i = 0; i < 3; i++){
            string fn = "../assets/fallout_car_2/scene.gltf";
            AssimpLoader* ld = new AssimpLoader();
            Object* nobj = ld->LoadFile(fn);

            nobj->Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, -50.0 * i)));

            DEngine::gobjs.push_back(nobj);
        }

        if(!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch(DxException& e)
    {
        MessageBoxW(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}