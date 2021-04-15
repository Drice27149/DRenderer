#version 330 core
struct ModelTexture{
	sampler2D baseColor;
	sampler2D normal;
    sampler2D metallicRoughness;
};

struct Light{
	vec3 pos;
	vec3 color;				    
};

uniform ModelTexture modelTex;	
uniform Light light0;			// 默认光源
uniform vec3 viewPos;			// 相机位置
uniform float gloss;

const float PI = 3.1415926;

in vec2 uv;
in vec3 worldPos;
in mat3 TBN;

out vec4 FragColor;

vec3 GetNormal()
{
	vec3 n = texture(modelTex.normal, uv).rgb;
	n = normalize(n*2.0 - 1.0);
	n = normalize(TBN * n);
	return n;
}

vec3 GetLightDir()
{
	return normalize(vec3(-1.0, 1.0, 1.0));
}

float distributionGGX (vec3 N, vec3 H, float roughness){
    float a2    = roughness * roughness * roughness * roughness;
    float NdotH = max (dot (N, H), 0.0);
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float geometrySchlickGGX (float NdotV, float roughness){
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith (vec3 N, vec3 V, vec3 L, float roughness){
    return geometrySchlickGGX (max (dot (N, L), 0.0), roughness) * 
           geometrySchlickGGX (max (dot (N, V), 0.0), roughness);
}

vec3 fresnelSchlick (float cosTheta, vec3 F0){
    return F0 + (1.0 - F0) * pow (1.0 - cosTheta, 5.0);
}

void main(){
    vec3 albedo = pow(texture(modelTex.baseColor, uv).rgb, vec3(2.2));
    float ao = texture(modelTex.metallicRoughness, uv).r;
    float metallic = texture(modelTex.metallicRoughness, uv).b;
    float roughness = texture(modelTex.metallicRoughness, uv).g;

    vec3 N = normalize(GetNormal());
    vec3 V = normalize(viewPos - worldPos);
    vec3 L = normalize(GetLightDir());
    vec3 H = normalize (V + L);
                
    // Cook-Torrance BRDF
    vec3  F0 = mix (vec3 (0.04), albedo, metallic);
    float NDF = distributionGGX(N, H, roughness);
    float G   = geometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);        
    vec3  kD  = vec3(1.0) - F;
    kD *= 1.0 - metallic;
                
    vec3  numerator   = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3  specular    = numerator / max(denominator, 0.001);  
                    
    float lightCC = 3.0;

    float NdotL = max(dot(N, L), 0.0);                
    vec3  color = lightCC * (kD*albedo/PI + specular) * (NdotL);
    vec3 ambient = vec3(0.03) * albedo * ao;
    color = color + ambient;

    color = pow(color/(color + 1.0), vec3(1.0/2.2));
   
    FragColor = vec4(color, 1.0);
}