#include "CamMgr.hpp"

CamMgr::CamMgr()
{

}

void CamMgr::ZoomCam(float dz)
{
    camera.zoom(dz);
}

void CamMgr::MoveCam(float dx, float dy)
{
    if(std::abs(dx) > std::abs(dy)) camera.moveX(-dx);
    else camera.moveY(dy);
}

void CamMgr::RotateCam(float dx, float dy)
{
    camera.rotatePitch(-dy);
    camera.rotateYaw(dx);
}

mat4 CamMgr::GetViewTransform()
{
    return camera.getCamTransform();
}

mat4 CamMgr::GetProjectionTransform()
{
    return camera.getProjTransform();
}

vec3 CamMgr::GetViewPos()
{
    return camera.getOrigin();
}