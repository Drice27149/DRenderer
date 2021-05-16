
#include "Global.hpp"
#include "Object.hpp"
#include "AssimpLoader.hpp"
#include "DEngine.hpp"
#include "Graphics.hpp"
#include "Light.hpp"
#include "Parallel.hpp"

Object* DEngine::gobj = nullptr;
DEngine* DEngine::instance = nullptr;
std::vector<Object*> DEngine::gobjs;

std::vector<metaData> Object::reflection = 
{
    metaData("scale", offsetof(Object, scale)),
    metaData("x",offsetof(Object, x)),
    metaData("y",offsetof(Object, y)),
    metaData("z",offsetof(Object, z)),
    metaData("pitch",offsetof(Object, pitch)),
    metaData("yaw",offsetof(Object, yaw)),
    metaData("roll",offsetof(Object, roll)),
};

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

        // @TODO: 基于配置的场景加载
        DEngine::gobjs.clear();

        // vector<string> fns = { "../assets/models/LightShapes/sphere.obj", "../assets/models/LightShapes/cube.obj", "../assets/models/LightShapes/sphere.obj" };

        // AssimpLoader ld;

        // for(int i = 0; i < 1; i++){
        //     string fn = "../assets/models/hulkbuster/scene.gltf";
        //     // string fn = fns[i];
           
        //     Object* nobj = new Object();
        //     ld.LoadFile(nobj, fn);

        //     nobj->Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0, 1.8, 0.0)));
        //     nobj->drawType = DrawType::Normal;
        //     nobj->Scale(0.02);
        //     nobj->Transform(glm::rotate(glm::mat4(1.0), 0.5f*3.1415926f, glm::vec3(1.0, 0.0, 0.0)));
        //     nobj->Transform(glm::rotate(glm::mat4(1.0), 3.1415926f, glm::vec3(0.0, 0.0, 1.0)));

        //     DEngine::gobjs.push_back(nobj);
        // }

        std::shared_ptr<Object> hulk = std::make_shared<Object>();
        hulk->Scale(0.02);

        objJobs.emplace_back(ObjJob("../assets/models/hulkbuster/scene.gltf", hulk));

        auto WorkFunc = [](int64_t id)->void{
            auto& job = objJobs[id];
            AssimpLoader ld;
            ld.LoadFile(job.obj.get(), job.name);
        };

        parallelInit();
        parallelFor1D(WorkFunc, objJobs.size(), 1);
        parallelCleanUp();

        for(auto& job: objJobs){
             DEngine::gobjs.push_back(job.obj.get());
        }

        // debug cluster
        // Object cluster;
        // cluster.meshes.push_back(Frustum(45.0, 1.0, 1.0, 20.0, 16, 8, 4));
        // cluster.drawType = DrawType::WhiteLines;
        // cluster.Transform(glm::translate(glm::mat4(1.0), glm::vec3(0.0,3.0,12.0)));
        // DEngine::gobjs.push_back(&cluster);

        Object grid;
        grid.meshes.push_back(Grid());
        grid.drawType = DrawType::WhiteLines;
        DEngine::gobjs.push_back(&grid);

        // @TODO: fall back texture for no uv or no baseColor
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