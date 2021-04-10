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
    // ��ת
    if(lmouse){
        DEngine::GetCamMgr().RotateCam(dx, dy);
    }
    // ƽ��
    if(rmouse){
        DEngine::GetCamMgr().MoveCam(dx, dy);
    }
}