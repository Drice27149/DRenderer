#include "Object.hpp"

Object::Object()
{
	mask = 0;

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