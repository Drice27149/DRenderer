
#include "Global.hpp"
#include "Object.hpp"
#include "AssimpLoader.hpp"
#include "DEngine.hpp"
#include "Graphics.hpp"

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

        Graphics theApp(hInstance);

        DEngine::gobjs.clear();

        vector<string> fns = { "../assets/LightShapes/sphere.obj", "../assets/LightShapes/cube.obj", "../assets/LightShapes/sphere.obj" };

        for(int i = 0; i < 1; i++){
            // string fn = "../assets/corvette_stingray/scene.gltf";
            string fn = fns[i];
            AssimpLoader* ld = new AssimpLoader();
            Object* nobj = ld->LoadFile(fn);

            nobj->Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 1.8, 0.0)));
            nobj->drawType = DrawType::Normal;

            DEngine::gobjs.push_back(nobj);
        }

        // Object cluster;
        // cluster.meshes.push_back(Frustum(45.0, 1.0, 1.0, 10.0, -1,-1,-1));
        // cluster.drawType = DrawType::WhiteLines;

        // DEngine::gobjs.push_back(&cluster);

        Object panel;
        panel.meshes.push_back(Panel());
        panel.drawType = DrawType::Normal;
        DEngine::gobjs.push_back(&panel);

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