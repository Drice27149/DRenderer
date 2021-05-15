#pragma once

#include "Global.hpp"
#include "InputMgr.hpp"
#include "CamMgr.hpp"
#include "SceneMgr.hpp"
#include "Object.hpp"

class DEngine{
public:
    DEngine();
    static void Launch();
    static InputMgr& GetInputMgr(){ return instance->inputMgr; }
    static CamMgr& GetCamMgr(){ return instance->camMgr; }
    static SceneMgr& GetSceneMgr(){ return instance->sceneMgr; }
    static void Tick();
    static void LogError(string s);
public:
    static DEngine* instance;
    static GLFWwindow* window;
    static Object* gobj;
    static vector<Object*> gobjs;
    static vector<Mesh*> meshes;
    CamMgr camMgr;
    InputMgr inputMgr;
    SceneMgr sceneMgr;
};