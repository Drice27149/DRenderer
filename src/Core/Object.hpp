#pragma once

#include "Global.hpp"
#include "Mesh.hpp"
#include "Struct.hpp"
#include "Reflect.hpp"
#include "WorldTriangle.hpp"

enum DrawType {
    Normal,
    WhiteLines,
    RedLines,
	SpotLgiht, 	// frustum
	PointLight,	// sphere
	Debug,
};

class Object {
public:
	Object();
	void Transform(mat4 trans);
	void Scale(float rate);
	void MergeMesh(Mesh& mesh);
	mat4 GetModelTransform();
	void deserialize(std::vector<Reflect::Data> datas);
	std::vector<Reflect::Data> serialize();
	std::vector<WorldTriangle> GetTriangleList();
public:
	// transform data
	string fn;
	float scale;
	float pitch;
	float yaw;
	float roll;
	// position
	float x;
	float y;
	float z;
	float metallic;
	float roughness;
	// color
	float cx;
	float cy;
	float cz;
	// scale
	float sx;
	float sy;
	float sz;
public:
	// render data
	int mask;
	// @TODO: shading model
	// @TODO: material data
	vector<Mesh> meshes;
public:
	mat4 model;
	DrawType drawType;
	int id;
};

static std::vector<Object*> waitLoadObj;