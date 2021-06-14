#include "Camera.hpp"
#include <cmath>

Camera::Camera(vec3 origin, float rotateSpeed, float moveSpeed):
origin(origin), rotateSpeed(rotateSpeed), moveSpeed(moveSpeed)
{
    yaw = 90.0;
    pitch = 30.0;
    radius = sqrt(glm::dot(origin, origin));
    position = origin;
    offset = vec3(0.0, 0.0, 0.0);
}

void Camera::moveX(float direction){
    offset.x += direction;
}

void Camera::moveY(float direction){
    offset.y += direction;
}

void Camera::moveZ(float direction){
    offset.z += direction;
}

void Camera::rotate(float dx, float dy){
}

mat4 Camera::getCamTransform(){
    updatePosition();
    vec3 z = glm::normalize(position);
    vec3 y = glm::normalize(glm::vec3(0.0, 1.0, 0.0));
    vec3 x = glm::normalize(glm::cross(y, z));
    vec3 realY = glm::normalize(glm::cross(z, x));
    glm::mat4 res =  glm::lookAt(position, glm::vec3(0.0, 0.0, 0.0), realY);
    res = glm::translate(res, offset);
    return res;
}

void Camera::updateDirection(){
    gaze.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    gaze.y = sin(glm::radians(pitch));
    gaze.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    gaze = glm::normalize(gaze);
    vec3 worldUp = glm::normalize(glm::vec3(0, 1.0f, 0));
    vec3 right = glm::normalize(glm::cross(gaze, worldUp));
    up = glm::normalize(glm::cross(right, gaze));
}

vec3 Camera::getOrigin(){
    updatePosition();
    return position;
}

void Camera::SetMSpeed(float speed)
{
    this->moveSpeed = speed;
}

void Camera::SetRSpeed(float speed)
{
    this->rotateSpeed = speed;
}

void Camera::rotatePitch(float delta){
    pitch += delta * rotateSpeed;
    if(pitch < -89.0) 
        pitch = -89.0;
    if(pitch > 89.0)
        pitch = 89.0;
}

void Camera::rotateYaw(float delta){
    yaw += delta * rotateSpeed;
}

void Camera::zoom(float dir)
{
    radius += dir*moveSpeed;
}

void Camera::updatePosition()
{
    position.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    position.y = sin(glm::radians(pitch));
    position.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    position = glm::normalize(position) * radius;
}