#version 330 core
// layout (location = 0) in vec3 avCoord;
// layout (location = 1) in vec2 astCoord;
// layout (location = 2) in vec3 avNormal;
// layout (location = 3) in vec3 tangent;
// layout (location = 4) in vec3 bitangent;
// out vec3 bvCoord;
// out vec2 bstCoord;
// out vec3 bvNormal;
// out mat3 TBN;
// // projection * view
// uniform mat4 trans;
// // set up per object
// uniform mat4 normalTrans;
// uniform mat4 model;

// void main()
// {
// 	gl_Position = trans * model * vec4(avCoord.x, avCoord.y, avCoord.z, 1.0);
// 	bvCoord = (model * vec4(avCoord, 1.0)).xyz;
// 	bstCoord = astCoord;
// 	// adjust normal base on model tranform
// 	vec4 temp = normalTrans * vec4(avNormal, 1.0); 
// 	temp = temp / temp.w;
// 	bvNormal = temp.xyz;
// 	// tangent and bitangent
// 	vec3 T = normalize(vec3(model * vec4(tangent,   0.0)));
//    	vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
// 	vec3 N = normalize(cross(T, B));
// 	TBN = mat3(T, B, N);
// }

uniform mat4 model; 	// 模型变换
uniform mat4 view;		// 相机变换
uniform mat4 proj;		// 透视投影变换

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out vec2 uv;
out vec3 worldPos;
out mat3 TBN;

void main()
{
	gl_Position = proj * view * model * vec4(vertex, 1.0); 
	uv = texCoord;
	worldPos = vertex;
	vec3 worldNormal = inverse(transpose(mat3(model))) * (normal);
	TBN = mat3(normalize(tangent), normalize(bitangent), normalize(worldNormal));
}