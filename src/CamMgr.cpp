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
    camera.moveX(dx);
    camera.moveY(dy);
}

void CamMgr::RotateCam(float dx, float dy)
{
    camera.rotatePitch(dy);
    camera.rotateYaw(dx);
}