#include "CamMgr.hpp"

CamMgr::CamMgr()
{

}

void CamMgr::ZoomCam(float dz)
{
    camera.moveZ(dz);
}

void CamMgr::MoveCam(float dx, float dy)
{
    if(std::abs(dx) > std::abs(dy)) camera.moveX(dx);
    else camera.moveY(dy);
}

void CamMgr::RotateCam(float dx, float dy)
{
    camera.rotatePitch(dy);
    camera.rotateYaw(dx);
}