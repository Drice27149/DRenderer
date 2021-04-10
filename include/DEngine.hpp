#pragma once

#include "Global.hpp"
#include "InputMgr.hpp"
#include "CamMgr.hpp"

class DEngine{
public:
    DEngine();
    static void Launch();
    static InputMgr& GetInputMgr(){ return instance->inputMgr; }
    static CamMgr& GetCamMgr(){ return instance->camMgr; }
public:
    static DEngine* instance;
    CamMgr camMgr;
    InputMgr inputMgr;
};