
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
    metaData("metallic", offsetof(Object, metallic)),
    metaData("roughness", offsetof(Object, roughness)),
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

        // IBL测试0: 小球, 基于参数的metallicRoughness
        // int length = 100;
        // int count = 5;
        // float step = 1.0 / (float)(count-1);
        // int intStep = length / (count-1);
        // for(int i = 0; i < count; i++){
        //     for(int j = 0; j < count; j++){
        //         std::shared_ptr<Object> ball = std::make_shared<Object>();
        //         objJobs.emplace_back(ObjJob("../assets/models/sphere/scene.gltf", ball));
        //         ball->Scale(0.1);
        //         ball->roughness = (float)i * step;
        //         ball->metallic = (float)j * step;
        //         ball->x = i * intStep;
        //         ball->y = j * intStep;
        //     }
        // }
        // IBL测试1: 导入模型, 基于贴图的metallicRoughness
        std::shared_ptr<Object> hulk = std::make_shared<Object>();
        hulk->metallic = 0.8;
        hulk->roughness = 0.1;
        hulk->Scale(0.01);
        hulk->pitch = 90;
        hulk->yaw = 180;
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