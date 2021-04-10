#version 330 core
// in vec3 bvCoord;
// in vec2 bstCoord;
// in vec3 bvNormal;
// in mat3 TBN;
// layout (location = 0) out vec3 gPosition;
// layout (location = 1) out vec3 gNormal;
// layout (location = 2) out vec4 gColor;

// uniform sampler2D texture0; // set up per object
// uniform sampler2D depthMap; // not used
// uniform sampler2D normalMap; // not used
// // set up per scene
// uniform vec3 lightPos;
// uniform vec3 eyePos;
// uniform vec3 inten;
// uniform vec3 inten_a;

// vec3 GetNormal()
// {
//     vec3 normal = texture(normalMap, bstCoord).rgb;
// 	normal = normal * 2.0 - 1.0;
// 	normal = normalize(TBN * normal);
//     return normal;
// }

// vec4 blingFong()
// {
// 	vec4 k_s = vec4(0.9, 0.9, 0.9, 1.0);
// 	vec4 k_d = texture(texture0, bstCoord);
// 	vec4 k_a = k_d;
// 	vec4 i = vec4(inten, 1.0);
// 	vec4 i_a = vec4(inten_a, 1.0);
// 	float p = 32.0;
	
// 	// normal mapping
// 	vec3 normal = texture(normalMap, bstCoord).rgb;
// 	normal = normal * 2.0 - 1.0;
// 	normal = normalize(TBN * normal);

// 	vec3 n = normalize(bvNormal);
// 	vec3 v = normalize(eyePos - bvCoord);
// 	vec3 l = normalize(lightPos - bvCoord);
// 	vec3 h = normalize(l + v);
// 	float r = length(lightPos - bvCoord);

// 	float cosine0 = dot(l, n);
// 	if(cosine0 < 0.0) cosine0 = 0.0;
// 	vec4 specular = k_s * i/r/r * pow(dot(h, n), p);
// 	vec4 diffuse = k_d * i/r/r * cosine0;

// 	return k_d;
// 	return specular + diffuse;
// }

// void main()
// {
// 	gPosition = bvCoord;
//     gNormal = bvNormal;
//     gColor = blingFong();
// 	// FragColor = gColor;
// }
// Ä£ÐÍÎÆÀíÌùÍ¼
struct ModelTexture{
	sampler2D diffuse;
	sampler2D normal;
};

uniform ModelTexture modelTex;

in vec2 fvt;

out vec4 FragColor;

void main()
{
	FragColor = texture(modelTex.diffuse, fvt);
}