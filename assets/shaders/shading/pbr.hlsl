#include "brdf.hlsl"
#include "IBL.hlsl"

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	// float4x4 mvp = mul(mul(_model, _View), _Proj);
	float4x4 mvp = mul(mul(_Proj, _View), _model);
	vout.pos = mul(mvp, float4(vin.vertex, 1.0f));
	vout.worldPos = mul(_model, float4(vin.vertex, 1.0f)).rgb;

	float4x4 shadowMvp = mul(mul(_SMProj, _SMView), _model);
	vout.clipPos = mul(shadowMvp, float4(vin.vertex, 1.0f));
	// uv
	vout.uv = vin.texcoord;
	// in worldspace
    vout.T = mul(_model, float4(vin.tangent, 0.0)).rgb;
	vout.N = mul(_model, float4(vin.normal, 0.0)).rgb;
	// vout.T = vin.tangent;
	// vout.N = vin.normal;
    return vout;
}

float3 tangentToWorldNormal(float3 normal, float3 N, float3 T)
{
	N = normalize(N);
    T = normalize(T - dot(T, N)*(N));
    float3 B = cross(N, T);
    float3x3 TBN = transpose(float3x3(T, B, N));
	return normalize(mul(TBN, normal));
}

float3 GetLightDir()
{
	return normalize(float3(-1.0, 1.0, 1.0));
}

float SampleShadowMap(float4 clipPos)
{
	clipPos.xyz /= clipPos.w;
	float x = clipPos.x * 0.5 + 0.5;
	float y = (-clipPos.y) * 0.5 + 0.5;
	if(x>=0.0 && x<=1.0 && y>=0.0 && y<=1.0)
		return gShadowMap.Sample(gsamLinear, float2(x,y));
	else
		return 1.0;
}

float GetShadowBlur(float4 clipPos)
{
	clipPos.xyz /= clipPos.w;
	float x = clipPos.x * 0.5 + 0.5;
	float y = (-clipPos.y) * 0.5 + 0.5;
	if(x>=0.0 && x<=1.0 && y>=0.0 && y<=1.0){
		float depth = gShadowMap.Sample(gsamLinear, float2(x,y));
		if(clipPos.z < depth + 0.005) 
			return 0.0;
		else 
			return 1.0;
	}
	else
		return 0.0;
}

float4 shading(vec3 n, vec3 v, vec3 l, float2 uv, vec3 intensity)
{
	float3 baseColor = gDiffuseMap.Sample(gsamLinear, uv).rgb;
	float roughness = gMetallicMap.Sample(gsamLinear, uv).g;
	float metallic = gMetallicMap.Sample(gsamLinear, uv).b;
	float3 diffuseColor = baseColor; // (1.0 - metallic) * baseColor;
	float3 f0 = baseColor * metallic;
	float a = roughness*roughness;
	float3 color = BRDF(n, v, l, diffuseColor, a, f0, roughness) * dot(n, l);
	color = color * intensity;
	float3 ambient = baseColor * float3(0.03, 0.03, 0.03);
	float ao = gMetallicMap.Sample(gsamLinear, uv).r;
	color = color * ambient * ao;
	return float4(color, 1.0);
}

float4 previousWork(VertexOut pin)
{
	float3 normal = gNormalMap.Sample(gsamLinear, pin.uv).rgb;
	normal = normal*2.0 - 1.0;
	normal = tangentToWorldNormal(normal, pin.N, pin.T);
	// normal = normalize(pin.N);
	float3 viewDir = normalize(_CamPos - pin.worldPos);
	float3 lightDir = GetLightDir();

	float4 color = shading(normal, viewDir, lightDir, pin.uv, 3.5);

	return color;
}

float4 pbrSphereTest(VertexOut pin)
{
	float3 intensity = float3(2.0, 2.0, 2.0);

	float3 n = normalize(pin.N);
	float3 v = normalize(_CamPos - pin.worldPos);
	float3 l = normalize(GetLightDir());

	float3 baseColor = float3(1.0, 1.0, 1.0);
	float metallic = _metallic;
	float roughness = _roughness;
	float3 f0 = baseColor * metallic;
	float a = roughness*roughness;

	float3 color = BRDF(n, v, l, baseColor, a, f0, roughness) * intensity * saturate(dot(n, l));
	float3 ambient = float3(0.1, 0.1, 0.1);
	color = color + ambient;

	return float4(color, 1.0);
}

float4 pureIBL(VertexOut pin)
{
	float3 baseColor = float3(1.0, 1.0, 1.0);
	float3 fd = diffuseIBL(normalize(pin.N), (1.0-_metallic)*baseColor, 128);
	float3 fr = specularIBL(normalize(pin.N), normalize(_CamPos - pin.worldPos), baseColor, _metallic, _roughness, 128);
	return float4(fd + fr, 1.0);
	return gCubeMap.Sample(gsamLinear, normalize(pin.N));
}

float3 DirectLight(float3 N, float3 V, float3 L, float3 baseColor, float roughness, float metallic)
{
	float NoL = saturate(dot(N, L));
	if(NoL <= 0.0)
	{
		return float3(0.0, 0.0, 0.0);
	}
	float3 lighting = BRDF_Faliment(N, V, L, baseColor, roughness, metallic) * NoL;
	return lighting;
}

float4 PS(VertexOut pin) : SV_Target
{
	// float3 color = float3(1.0, 1.0, 1.0);
	// color = color * dot(normalize(pin.N),normalize(GetLightDir()));
	float3 baseColor = gDiffuseMap.Sample(gsamLinear, pin.uv).rgb;
	float ao = gMetallicMap.Sample(gsamLinear, pin.uv).r;
	float roughness = gMetallicMap.Sample(gsamLinear, pin.uv).g;
	float metallic = gMetallicMap.Sample(gsamLinear, pin.uv).b;
	float3 normal = gNormalMap.Sample(gsamLinear, pin.uv).rgb;
	normal = normal*2.0 - 1.0;
	normal = tangentToWorldNormal(normal, pin.N, pin.T);
	if(!(_mask & (1<<6))){
		normal = pin.N;
	}

	float3 N = normalize(normal);
	float3 V = normalize(_CamPos - pin.worldPos);
	float3 L = normalize(float3(_MainLightDirX, _MainLightDirY, _MainLightDirZ));

	baseColor = float3(1.0, 1.0, 1.0);
	roughness = _roughness;
	metallic = _metallic;

	float3 outColor = float3(0.0, 0.0, 0.0);
	outColor = outColor + DirectLight(N, V, L, baseColor, roughness, metallic) * _lightIntensity;
	outColor = outColor + AmbientIBL(N, V, baseColor, roughness, metallic) * _envIntensity;
	outColor = outColor + gEmissiveMap.Sample(gsamLinear, pin.uv).rrr;
	return float4(outColor, 1.0);
}