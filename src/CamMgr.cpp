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

mat4 CamMgr::GetViewTransform()
{
    return camera.getCamTransform();
}

mat4 CamMgr::GetProjectionTransform()
{
    // TODO: ���� unity ���������
    return glm::perspective(45.0, 1.0, 5.0, 2000.0);
}

vec3 CamMgr::GetViewPos()
{
    return camera.getOrigin();
}