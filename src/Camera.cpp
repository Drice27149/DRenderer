#include "Camera.hpp"
#include <iostream>
#include <cmath>

Camera::Camera(vec3 origin, vec3 firstGaze, float rotateSpeed, float moveSpeed):
origin(origin), firstGaze(firstGaze), rotateSpeed(rotateSpeed), moveSpeed(moveSpeed)
{
    firstGaze = glm::normalize(firstGaze);
    yaw = 0;
    pitch = 0;
}

void Camera::moveX(float direction){
    updateDirection();
    vec3 hv = glm::normalize(glm::cross(up, gaze));
    origin += hv * direction * moveSpeed;
}

void Camera::moveY(float direction){
    updateDirection();
    origin += up * direction * moveSpeed;
}

void Camera::moveZ(float direction){
    updateDirection();
    origin += gaze * direction * moveSpeed;
}

void Camera::rotate(float dx, float dy){

}

mat4 Camera::getCamTransform(){
    updateDirection();
    return glm::lookAt(origin, origin + gaze, up);
}

void Camera::rotatePitch(float delta){
    pitch += delta * rotateSpeed;
}

void Camera::rotateYaw(float delta){
    yaw += delta * rotateSpeed;
}

void Camera::updateDirection(){
    mat3 Rx(
        1.0f,          0,           0,
           0, cos(pitch), -sin(pitch),
           0, sin(pitch),  cos(pitch)
    ); 
    mat3 Ry(
        cos(yaw), 0, sin(yaw),
        0, 1.0f, 0,
        -sin(yaw), 0, cos(yaw)
    );
    gaze = Ry * Rx * firstGaze;
    vec3 worldUp = vec3(0, 1.0f, 0);
    vec3 right = glm::cross(gaze, worldUp);
    up = glm::cross(right, gaze);
}

// use input to adjust camera's postion and rotation
void Camera::processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        moveY(1.0f);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        moveY(-1.0f);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        moveX(1.0f);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        moveX(-1.0f);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        moveZ(1.0f);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        moveZ(-1.0f);

    float rspeed = 0.1;

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        rotateYaw(-1.0f * rspeed);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        rotateYaw(1.0f * rspeed);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        rotatePitch(1.0f * rspeed);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        rotatePitch(-1.0f * rspeed);
}

vec3 Camera::getOrigin(){
    updateDirection();
    return origin;
}