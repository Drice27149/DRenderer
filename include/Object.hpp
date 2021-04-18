#pragma once

#include "Global.hpp"
#include "Mesh.hpp"

class Object{
public:
	Object();
	vector<Mesh> meshes;
	void Transform(mat4 trans);
public:
	mat4 model;
};