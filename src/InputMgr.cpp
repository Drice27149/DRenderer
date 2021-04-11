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
        DEngine::GetCamMgr().RotateCam(dx, -dy);
    }
    // Æ½ÒÆ
    if(rmouse){
        DEngine::GetCamMgr().MoveCam(-dx, -dy);
    }
}

void InputMgr::OnZoomIn()
{
    DEngine::GetCamMgr().ZoomCam(1.0);
}

void InputMgr::OnZoomOut()
{
    DEngine::GetCamMgr().ZoomCam(-1.0);
}

void InputMgr::OnMoveLeft()
{
    DEngine::GetCamMgr().MoveCam(1.0, 0);
}

void InputMgr::OnMoveRight()
{
    DEngine::GetCamMgr().MoveCam(-1.0, 0);
}