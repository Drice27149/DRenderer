#pragma once

#include "Global.hpp"
#include "Mesh.hpp"
#include "Struct.hpp"
#include "Reflect.hpp"

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
	void Transform(mat4 trans);
	void Scale(float rate);
	void MergeMesh(Mesh& mesh);
	mat4 GetModelTransform();
	void deserialize(std::vector<Reflect::Data> datas);
	std::vector<Reflect::Data> serialize();
public:
	// transform data
	float scale;
	float pitch;
	float yaw;
	float roll;
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