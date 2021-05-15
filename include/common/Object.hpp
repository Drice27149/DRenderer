#pragma once

#include "Global.hpp"
#include "Mesh.hpp"

enum DrawType {
    Normal,
    WhiteLines,
    RedLines,
	SpotLgiht, 	// frustum
	PointLight	// sphere
};

class Object {
public:
	Object();
	vector<Mesh> meshes;
	void Transform(mat4 trans);
	void Scale(float rate);
	void MergeMesh(Mesh& mesh);
public:
	// 材质贴图 mask, 若存在相应的贴图则对应位置为1
	unsigned int mask;
	std::vector<std::string> texns;
public:
	mat4 model;
	DrawType drawType;
	int id;
};