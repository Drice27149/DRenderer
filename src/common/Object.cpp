#include "Object.hpp"

Object::Object()
{
	mask = 0;

	model = mat4(1.0);
	drawType = DrawType::Normal;

	scale = 1.0;
	x = y = z = 0.0;
	pitch = roll = yaw = 0.0;
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
	}
}

mat4 Object::GetModelTransform()
{
	mat4 result = glm::mat4(1.0);
	// scale first, only support equal scale
	result = glm::scale(result, vec3(scale, scale, scale));
	// rotation second
	result = glm::rotate(result, glm::radians(pitch), vec3(1.0, 0.0, 0.0));
	result = glm::rotate(result, glm::radians(yaw), vec3(0.0, 1.0, 0.0));
	result = glm::rotate(result, glm::radians(pitch), vec3(0.0, 0.0, 1.0));
	// translation last
	result = glm::translate(result, vec3(x, y, z));

	return result;
}