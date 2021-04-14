#version 330 core

uniform samplerCube skybox;

in vec3 uv;

out vec4 FragColor;

void main()
{
	FragColor = texture(skybox, uv);
}