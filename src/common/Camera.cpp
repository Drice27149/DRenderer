#include "Camera.hpp"

Camera::Camera(vec3 origin, float rotateSpeed, float moveSpeed):
origin(origin), rotateSpeed(rotateSpeed), moveSpeed(moveSpeed)
{
    yaw = -90.0;
    pitch = 0.0;
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
    if(pitch < -89.0) 
        pitch = -89.0;
    if(pitch > 89.0)
        pitch = 89.0;
}

void Camera::rotateYaw(float delta){
    yaw += delta * rotateSpeed;
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
    updateDirection();
    return origin;
}

void Camera::SetMSpeed(float speed)
{
    this->moveSpeed = speed;
}

void Camera::SetRSpeed(float speed)
{
    this->rotateSpeed = speed;
}