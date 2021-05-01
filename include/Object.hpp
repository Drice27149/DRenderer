#pragma once

#include "Global.hpp"
#include "Mesh.hpp"

enum DrawType {
    Normal,
    WhiteLines,
    RedLines,
	LightSource
};

class Object{
public:
	Object();
	vector<Mesh> meshes;
	void Transform(mat4 trans);
public:
	mat4 model;
	DrawType drawType;
};