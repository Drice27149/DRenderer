#version 330 core

uniform mat4 model; 	// ģ�ͱ任
uniform mat4 view;		// ����任
uniform mat4 proj;		// ͸��ͶӰ�任

layout (location = 0) in vec3 v;

void main()
{
	gl_Position = proj * view * model * vec4(v.xyz, 1.0); 
}