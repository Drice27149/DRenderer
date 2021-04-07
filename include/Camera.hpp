#pragma once

#include "Global.hpp"
using glm::vec3;
using glm::mat4;
using glm::mat3;

class Camera {
public:
    Camera(vec3 origin, vec3 firstGaze, float rSpeed = 0.0f, float mSpeed = 0.0f);
    void moveX(float direciton); 
    void moveY(float direction); 
    void moveZ(float direction); 
    void rotate(float dyaw, float dpitch);
    void rotatePitch(float delta);
    void rotateYaw(float delta);
    void updateDirection();
    void processInput(GLFWwindow*);
    mat4 getCamTransform();
    vec3 getOrigin();
private:
    vec3 firstGaze;
    vec3 origin;
    vec3 gaze;
    vec3 up;
    float yaw;
    float pitch;
    float moveSpeed;
    float rotateSpeed;
};