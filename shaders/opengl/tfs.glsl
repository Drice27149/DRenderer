#version 330 core
struct ModelTexture{
	sampler2D baseColor;
	sampler2D normal;
	sampler2D metallicRoughness;
};

struct Light{
	vec3 pos;
	vec3 color;					// lightmode 使用宏会比较恰当
};

uniform ModelTexture modelTex;	
uniform Light light0;			// 默认光源
uniform vec3 viewPos;			// 相机位置
uniform float gloss;

in vec2 uv;
in vec3 worldPos;
in mat3 TBN;

out vec4 FragColor;

float GetAtten()
{
	return 1.0;
}

vec3 GetNormal()
{
	vec3 n = texture(modelTex.normal, uv).rgb;
	n = n*0.5 + 0.5;
	n = normalize(TBN * n);
	return n;
}

// 往表面外射出的方向
vec3 GetLightDir()
{
	return normalize(vec3(-1.0, 1.0, 1.0));
}

float GetLightAmbient()
{
	return 0.2;
}

vec4 BlingFong()
{
	vec3 albedo = vec3(1.0); // texture(modelTex.diffuse, uv).rgb;
	vec3 normal = GetNormal();
	vec3 viewDir = normalize(viewPos - worldPos);
	vec3 lightDir = normalize(GetLightDir());
	vec3 halfDir = normalize(lightDir + viewDir);
	vec3 specular = light0.color *  pow(max(0.0, dot(normal, halfDir)), gloss);
	vec3 diffuse = light0.color * albedo * max(0, dot(normal, lightDir));
	vec3 ambient = GetLightAmbient() * albedo;
	float atten = GetAtten();
	return vec4((specular + diffuse)*atten + ambient, 1.0);
}

void main()
{
	vec4 blColor = BlingFong();
	FragColor = blColor;
}