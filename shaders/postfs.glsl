#version 330 core
in vec3 bvCoord;
in vec2 bstCoord;
in vec3 bvNormal;
out vec4 FragColor;
uniform sampler2D depthMap;
uniform mat4 lightTrans;

void main()
{
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}