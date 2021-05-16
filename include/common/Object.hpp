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

struct metaData {
	std::string name;
	unsigned long long offset;
	metaData(std::string name, unsigned long long offset):name(name), offset(offset){}
};

class Object {
public:
	Object();
	void Transform(mat4 trans);
	void Scale(float rate);
	void MergeMesh(Mesh& mesh);
	mat4 GetModelTransform();
public:
	// transform data
	float scale;
	float roll;
	float pitch;
	float yaw;
	float x;
	float y;
	float z;
	float metallic;
	float roughness;
public:
	// render data
	unsigned int mask;
	std::vector<std::string> texns;
	// @TODO: shading model
	// @TODO: material data
	vector<Mesh> meshes;
	// @TODO: reflection 
	static std::vector<metaData> reflection;
public:
	mat4 model;
	DrawType drawType;
	int id;
};

struct ObjJob {
	std::string name;
	std::shared_ptr<Object> obj;
	ObjJob(std::string name, std::shared_ptr<Object> obj):
	name(name), obj(obj)
	{}
};

static std::vector<ObjJob> objJobs;