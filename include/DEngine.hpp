#pragma once

#include "Global.hpp"
#include "InputMgr.hpp"
#include "CamMgr.hpp"
#include "TextureMgr.hpp"
#include "RenderMgr.hpp"
#include "SceneMgr.hpp"

class DEngine{
public:
    DEngine();
    static void Launch();
    static InputMgr& GetInputMgr(){ return instance->inputMgr; }
    static CamMgr& GetCamMgr(){ return instance->camMgr; }
    static TextureMgr& GetTexMgr() { return instance->texMgr; }
    static RenderMgr& GetRenderMgr(){ return instance->renderMgr; }
    static SceneMgr& GetSceneMgr(){ return instance->sceneMgr; }
    static void Tick();
public:
    static DEngine* instance;
    static GLFWwindow* window;
    CamMgr camMgr;
    InputMgr inputMgr;
    TextureMgr texMgr;
    RenderMgr renderMgr;
    SceneMgr sceneMgr;
};