#version 330 core

uniform mat4 model; 	// 模型变换
uniform mat4 view;		// 相机变换
uniform mat4 proj;		// 透视投影变换

layout (location = 0) in vec3 v;

void main()
{
	gl_Position = proj * view * model * vec4(v.xyz, 1.0); 
}