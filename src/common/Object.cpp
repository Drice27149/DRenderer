#include "Object.hpp"

Object::Object()
{
	model = mat4(1.0);
	drawType = DrawType::Normal;
}

void Object::Transform(mat4 trans)
{
	model = trans * model;
}

void Object::Scale(float rate)
{
	glm::mat4 scaleM = glm::mat4(1.0);
	for(int i = 0; i < 3; i++) scaleM[i][i] = rate;
	
	model = model * scaleM;
}