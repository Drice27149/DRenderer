
#include "Global.hpp"
#include "Object.hpp"
#include "AssimpLoader.hpp"
#include "DEngine.hpp"
#include "Graphics.hpp"
#include "Light.hpp"

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

        AssimpLoader ld;

        for(int i = 0; i < 1; i++){
            // string fn = "../assets/corvette_stingray/scene.gltf";
            string fn = fns[i];
           
            Object* nobj = new Object();
            ld.LoadFile(nobj, fn);

            nobj->Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 1.8, 0.0)));
            nobj->drawType = DrawType::Normal;

            DEngine::gobjs.push_back(nobj);
        }

        // debug cluster
        Object cluster;
        cluster.meshes.push_back(Frustum(45.0, 1.0, 1.0, 20.0, 16, 8, 4));
        cluster.drawType = DrawType::WhiteLines;
        cluster.Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0,3.0,12.0)));
        DEngine::gobjs.push_back(&cluster);

        // Object panel;
        // panel.meshes.push_back(Panel());
        // panel.drawType = DrawType::Normal;
        // DEngine::gobjs.push_back(&panel);

        Light pointLight(DrawType::PointLight);
        pointLight.Transform(glm::translate(glm::mat4(1.0), glm::vec3(-5.0, 2.5, 0.0)));
        pointLight.Scale(3.0);
        pointLight.id = 0;
        DEngine::gobjs.push_back(&pointLight);

        Light pointLight0(DrawType::PointLight);
        pointLight0.Transform(glm::translate(glm::mat4(1.0), glm::vec3(5.0, 5.0, 0.0)));
        pointLight0.id = 1;
        pointLight0.Scale(2.0);
        DEngine::gobjs.push_back(&pointLight0);

        Light pointLight1(DrawType::PointLight);
        pointLight1.Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 7.5, 0.0)));
        pointLight1.id = 2;
        pointLight1.Scale(1.0);
        DEngine::gobjs.push_back(&pointLight1);

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