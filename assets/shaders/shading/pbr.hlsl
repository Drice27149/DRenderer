#include "common.hlsl"
#include "brdf.hlsl"

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	float4x4 mvp = mul(mul(_model, _View), _Proj);

	vout.pos = mul(float4(vin.vertex, 1.0f), mvp);

	float4x4 shadowMvp = mul(mul(_model, _SMView), _SMProj);
	vout.clipPos = mul(float4(vin.vertex, 1.0f), shadowMvp);
	// uv
	vout.uv = vin.texcoord;
    vout.T = vin.tangent;
	vout.N = vin.normal;
    return vout;
}

float3 tangentToWorldNormal(float3 normal, float3 N, float3 T)
{
	N = normalize(N);
    T = normalize(T - dot(T, N)*(N));
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);
	return mul(TBN, normal);
}

float3 GetLightDir()
{
	return float3(-1.0, 1.0, 1.0);
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

float4 PS(VertexOut pin) : SV_Target
{
	float3 baseColor = gDiffuseMap.Sample(gsamLinear, pin.uv).rgb;
	return float4(baseColor, 1.0);
	// baseColor = float4(1.0, 1.0, 1.0, 1.0);
	float3 normal = gNormalMap.Sample(gsamLinear, pin.uv).rgb;
	normal = normal*2.0 - 1.0;
	normal = tangentToWorldNormal(normal, pin.N, pin.T);
	normal = normalize(pin.N);
	float3 lightDir = normalize(GetLightDir());
	float lightRate = dot(lightDir, normal);

	baseColor =	float4((baseColor) * lightRate, 1.0);

	float shadowBlur = GetShadowBlur(pin.clipPos);
	// lightRate = lightRate * (1.0 - shadowBlur);

	return float4(baseColor, 1.0);
}