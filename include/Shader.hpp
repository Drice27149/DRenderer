#pragma once

#include "Global.hpp"

class Shader {
public:
	// TODO: Ѕвсо vs, fs µД attach єН link
	Shader(std::string vertexShader, std::string fragmentShader);
	void setInt(const char* name, int value);
	void setFloat(const char* name, float value);
	void setVec3(const char* name, glm::vec3);
	void setMat4(const char* name, glm::mat4);
	void setVec2(const char* name, glm::vec2);
	void use();
	int getID(){ return programID; }
	int getUniformID(const char*);
private:
	int programID;
	unsigned int vertexID;
	unsigned int fragmentID;
};