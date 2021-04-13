#pragma once

#include "Global.hpp"
#include "InputMgr.hpp"
#include "CamMgr.hpp"
#include "TextureMgr.hpp"

class DEngine{
public:
    DEngine();
    static void Launch();
    static InputMgr& GetInputMgr(){ return instance->inputMgr; }
    static CamMgr& GetCamMgr(){ return instance->camMgr; }
    static TextureMgr& GetTexMgr() { return instance->texMgr; }
public:
    static DEngine* instance;
    CamMgr camMgr;
    InputMgr inputMgr;
    TextureMgr texMgr;
};