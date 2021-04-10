#pragma once

#include "Global.hpp"
#include "Camera.hpp"

class CamMgr{
public:
    CamMgr();
    void ZoomCam(float dz);
    void MoveCam(float dx, float dy);
    void RotateCam(float dx, float dy);
    Camera& GetCamera(){ return camera; };
private:
    Camera camera;
};