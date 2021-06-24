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

mat4 Camera::getProjTransform()
{
    return glm::perspective(glm::radians(fov), aspect, zNear, zFar);
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

void Camera::deserialize(std::vector<Reflect::Data> datas)
{
	for(const auto& data: datas){
		if(data.type == Reflect::Type::FLOAT){
			float* ptr = (float*)((unsigned long long)this + data.offset);
			*ptr = data.f[0];
		}
		if(data.type == Reflect::Type::FLOAT3){
			float* ptr0 = (float*)((unsigned long long)this + data.offset);
			float* ptr1 = (float*)((unsigned long long)this + data.offset + sizeof(float));
			float* ptr2 = (float*)((unsigned long long)this + data.offset + 2*sizeof(float));
			*ptr0 = data.f[0];
			*ptr1 = data.f[1];
			*ptr2 = data.f[2];
		}
		if(data.type == Reflect::Type::INT){
			int* ptr = (int*)((unsigned long long)this + data.offset);
			*ptr = data.i[0];
		}
		if(data.type == Reflect::Type::STRING){
			string* ptr = (string*)((unsigned long long)this + data.offset);
			*ptr = data.s;
		}
	}
}

std::vector<Reflect::Data> Camera::serialize()
{
    // yaw, pitch, radiance, position, offset
	std::vector<Reflect::Data> res = {
		// offset, type, name[3], data[3]
		Reflect::Data{offsetof(Camera, yaw), Reflect::Type::FLOAT, "Yaw", "", "", yaw},
		Reflect::Data{offsetof(Camera, pitch), Reflect::Type::FLOAT, "Pitch", "", "", pitch},
		Reflect::Data{offsetof(Camera, radius), Reflect::Type::FLOAT, "Radius", "", "", radius},
		Reflect::Data{offsetof(Camera, position), Reflect::Type::FLOAT3, "x", "y", "z", position[0], position[1], position[2]},
		Reflect::Data{offsetof(Camera, offset), Reflect::Type::FLOAT3, "offsetX", "offsetY", "offsetZ", offset[0], offset[1], offset[2]},
        Reflect::Data{offsetof(Camera, fov), Reflect::Type::FLOAT, "fov", "", "", fov},
        Reflect::Data{offsetof(Camera, aspect), Reflect::Type::FLOAT, "aspect", "", "", aspect},
        Reflect::Data{offsetof(Camera, zNear), Reflect::Type::FLOAT, "near", "", "", zNear},
        Reflect::Data{offsetof(Camera, zFar), Reflect::Type::FLOAT, "far", "", "", zFar},
        Reflect::Data{offsetof(Camera, lightPos), Reflect::Type::FLOAT3, "LihgtPosX", "LightPosY", "LightPosZ", lightPos[0], lightPos[1], lightPos[2]},
        Reflect::Data{offsetof(Camera, lightDir), Reflect::Type::FLOAT3, "LightDirX", "LightDirY", "LightDirZ", lightDir[0], lightDir[1], lightDir[2]},
        Reflect::Data{offsetof(Camera, lightColor), Reflect::Type::FLOAT3, "LightColorX", "LightColorY", "LightColorZ", lightColor[0], lightColor[1], lightColor[2]},
	};
	return res;
}

