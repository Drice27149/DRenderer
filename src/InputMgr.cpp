#include "InputMgr.hpp"
#include "CamMgr.hpp"
#include "DEngine.hpp"

InputMgr::InputMgr()
{
    lmouse = rmouse = false;
}

void InputMgr::Tick(float nx, float ny)
{
    float dx = nx-x;
    float dy = ny-y;
    x = nx; y = ny;
    // Ðý×ª
    if(lmouse){
        DEngine::GetCamMgr().RotateCam(dx, dy);
    }
    // Æ½ÒÆ
    if(rmouse){
        DEngine::GetCamMgr().MoveCam(dx, dy);
    }
}