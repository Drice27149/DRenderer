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