#pragma once

#include "Global.hpp"

class InputMgr{
public:
    InputMgr();

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
    float x, y;
    // ��һ֡��������
    float lx, ly;
    // ��ת
    bool lmouse;
    // λ��
    bool rmouse;
};