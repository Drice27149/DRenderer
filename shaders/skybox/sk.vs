#version 330 core

uniform mat4 model; 	// 模型变换
uniform mat4 view;		// 相机变换
uniform mat4 proj;		// 透视投影变换

layout (location = 0) in vec3 vertex;

out vec3 uv;

void main()
{
	gl_Position = proj * view * model * vec4(vertex.xyz, 1.0); 
    mat3 viewR = transpose(mat3(view));
    uv = viewR * vertex;
}