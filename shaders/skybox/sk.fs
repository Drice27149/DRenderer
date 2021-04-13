#version 330 core

uniform samplerCube skybox;

in vec3 worldPos;

out vec4 FragColor;

void main()
{
	FragColor = texture(skybox, worldPos);
}