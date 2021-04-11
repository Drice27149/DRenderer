#pragma once

#include "Global.hpp"

class InputMgr{
public:
    InputMgr();
    // 现在只能处理相机移动
    // TODO: 处理场景物体的位移, 旋转, 缩放
    void Tick(float nx, float ny);
    void OnLMouseDown(){ lmouse = true; }
    void OnLMouseRelease(){ lmouse = false; }
    void OnRMouseDown(){ rmouse = true; }
    void OnRMouseRelease(){ rmouse = false; }
    void OnZoomIn();
    void OnZoomOut();
    void OnMoveLeft();
    void OnMoveRight();
private:
    // 鼠标的坐标
    float x, y;
    // 上一帧鼠标的坐标
    float lx, ly;
    // 旋转
    bool lmouse;
    // 位移
    bool rmouse;
};