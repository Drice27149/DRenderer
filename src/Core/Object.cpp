#include "Object.hpp"

Object::Object()
{
	mask = 0;

	model = mat4(1.0);
	drawType = DrawType::Normal;

	scale = 1.0;
	x = y = z = 0.0;
	pitch = roll = yaw = 0.0;
	metallic = 0.0;
	roughness = 1.0;
	cx = cy = cz = 1.0;
	sx = sy = sz = 1.0;
}

void Object::Transform(mat4 trans)
{
	model = trans * model;
}

void Object::Scale(float rate)
{
	glm::mat4 scaleM = glm::mat4(1.0);
	for(int i = 0; i < 3; i++) scaleM[i][i] = rate;
	scale *= rate;
	
	model = model * scaleM;
}

void Object::MergeMesh(Mesh& mesh)
{
	meshes.push_back(mesh);
	/*
	if(meshes.size() == 0) 
		meshes.push_back(mesh);
	else{
		Mesh& org = meshes[0];
		int offvs = org.vs.size();
		int offids = org.ids.size();
		for(Vertex v: mesh.vs){
			org.vs.push_back(v);
		}
		for(unsigned int id: mesh.ids){
			org.ids.push_back(offvs + id);
		}
	}*/
}

mat4 Object::GetModelTransform()
{
	mat4 result = glm::mat4(1.0);
	// scale first, only support equal scale
	result[0][0] *= sx;
	result[1][1] *= sy;
	result[2][2] *= sz;
	for(int i = 0; i < 3; i++){
		result[i][i] *= scale;
	}
	// rotation second
	result = glm::rotate(result, glm::radians(pitch), vec3(1.0, 0.0, 0.0));
	result = glm::rotate(result, glm::radians(yaw), vec3(0.0, 1.0, 0.0));
	result = glm::rotate(result, glm::radians(roll), vec3(0.0, 0.0, 1.0));

	// translation last
	result[3] = result[3] + glm::vec4(x, y, z, 0.0);

	return result;
}

void Object::deserialize(std::vector<Reflect::Data> datas)
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

std::vector<Reflect::Data> Object::serialize()
{
	std::vector<Reflect::Data> res = {
		// offset, type, name[3], data[3]
		Reflect::Data{offsetof(Object, scale), Reflect::Type::FLOAT, "Scale", "", "", scale},
		Reflect::Data{offsetof(Object, x), Reflect::Type::FLOAT3, "X", "Y", "Z", x, y, z},
		Reflect::Data{offsetof(Object, pitch), Reflect::Type::FLOAT3, "Pitch", "Yaw", "Roll", pitch, yaw, roll},
		Reflect::Data{offsetof(Object, metallic), Reflect::Type::FLOAT, "Metallic", "", "", metallic},
		Reflect::Data{offsetof(Object, roughness), Reflect::Type::FLOAT, "Roughness", "", "", roughness},
		Reflect::Data{offsetof(Object, fn), Reflect::Type::STRING, "FileName", "", "", 0.0, 0.0, 0.0, 0, 0, 0, fn},
		Reflect::Data{offsetof(Object, cx), Reflect::Type::FLOAT3, "CX", "CY", "CZ", cx, cy, cz},
		Reflect::Data{offsetof(Object, sx), Reflect::Type::FLOAT3, "SX", "SY", "SZ", sx, sy, sz},
	};
	return res;
}