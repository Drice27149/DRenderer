#pragma once

#include "Global.hpp"

// TODO: w zoom in, s zoom out
// TODO: a/d 左右移动
// TODO: 左键位移, 右键旋转
class Camera {
public:
    Camera(vec3 origin = vec3(0, 0, 800)/*vec3(0, 3, 12)*/, float rSpeed = 0.4f, float mSpeed = 100.0f);
    void moveX(float direciton); 
    void moveY(float direction); 
    void moveZ(float direction); 
    void rotate(float dyaw, float dpitch);
    void rotatePitch(float delta);
    void rotateYaw(float delta);
    void updateDirection();
    void SetMSpeed(float speed);
    void SetRSpeed(float speed);
    mat4 getCamTransform();
    vec3 getOrigin();
    void zoom(float dir);
    void updatePosition();
public:
    vec3 firstGaze;
    vec3 origin;
    vec3 gaze;
    vec3 up;
    float yaw;
    float pitch;
    float moveSpeed;
    float rotateSpeed;
    float radius;
    vec3 position;
    vec3 offset;
};