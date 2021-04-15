#include "RenderMgr.hpp"
#include "GraphicAPI.hpp"
#include "Object.hpp"
#include "DEngine.hpp"

RenderMgr::RenderMgr()
{
    PreInit();
}

void RenderMgr::PreInit()
{
    if(bgrid){
        grid = new Grid();
        string l_vs = "../shaders/grid/line.vs";
        string l_fs = "../shaders/grid/line.fs";
        g_sh = new Shader(l_vs, l_fs);
    }
    
    if(bskybox){
        vector<string> fns = {
            "../assets/skybox/right.jpg",
            "../assets/skybox/left.jpg",
            "../assets/skybox/top.jpg",
            "../assets/skybox/bottom.jpg",
            "../assets/skybox/front.jpg",
            "../assets/skybox/back.jpg"
        };
        skybox = new CubeMap();
        GraphicAPI::LoadImageCubeMap(*skybox, fns);
        string c_vs = "../shaders/skybox/sk.vs";
        string c_fs = "../shaders/skybox/sk.fs";
        sk_sh = new Shader(c_vs, c_fs);
    }

    string b_vn = "../shaders/tvs.glsl";
    string b_fn = "../shaders/forward/pbr.glsl";

    base_sh = new Shader(b_vn, b_fn);
}

void RenderMgr::PrePass()
{
    if(bskybox){
        // GraphicAPI::Temp_DrawSkyBox(*skybox, *sk_sh);
    }

    if(bgrid){
        GraphicAPI::Temp_DrawGrid(*grid, *g_sh);
    }
}

void RenderMgr::BasePass()
{
    vector<Object*> objs = DEngine::GetSceneMgr().objs;
    for(Object* obj: objs){
        GraphicAPI::Temp_DrawObject(*obj, *base_sh);
    }
}

void RenderMgr::Render()
{
    GraphicAPI::BeforeRendering();

    PrePass();
    BasePass();

    GraphicAPI::AfterRendering();
}

