#include "BRDF.hlsl"

Texture2D diffuseMetallic: register(t0);
Texture2D normalRoughness: register(t1);
Texture2D worldPosX: register(t2);
Texture2D viewPosY: register(t3);
Texture2D shadowMap: register(t4);
TextureCube gEnvMap: register(t5);
Texture2D gBrdfMap: register(t6);

SamplerState gsamLinear: register(s0);

cbuffer LightDesc : register(b0)
{
    float3 _lightDir;
    float _lightInten;
    int _mainLight;
};

cbuffer RealPass : register(b1)
{
	float4x4 _View;
	float4x4 _Proj;
	float4x4 _SMView;
	float4x4 _SMProj;
	float4x4 _JProj;
	float4x4 _lastView;
	float4x4 _lastProj;
	float3 _CamPos;
};

struct VertexIn
{
	float3 vertex: POSITION;
};

struct VertexOut
{
	float4 pos: SV_POSITION;
};

VertexOut VS(VertexIn vin, uint id: SV_VertexID)
{
    VertexOut vout;
	if(id == 0 || id==5){
        vout.pos = float4(-1.0, 1.0, 0.0, 1.0);
    }
    else if(id==1){
        vout.pos = float4(1.0, 1.0, 0.0, 1.0);
    }
    else if(id==2 || id==3){
        vout.pos = float4(1.0, -1.0, 0.0, 1.0);
    }
    else{
        vout.pos = float4(-1.0, -1.0, 0.0, 1.0);
    }
    return vout;
}

float GetShadowBlur(float4 clipPos, float z)
{
	// clipPos.xyz /= clipPos.w;
	float x = clipPos.x/clipPos.w * 0.5 + 0.5;
	float y = (-clipPos.y/clipPos.w) * 0.5 + 0.5;
	if(x>=0.0 && x<=1.0 && y>=0.0 && y<=1.0){
		float depth = shadowMap.Sample(gsamLinear, float2(x,y)).r;
		if(z < depth + 0.005) 
			return 0.0;
		else 
			return 1.0;
	}
	else
		return 0.0;
}

float DecodeLOD(float roughness)
{
    float mipLevels = 4.0;
    return sqrt(roughness)*mipLevels;
}

float3 ApproximateSpecularIBL(float3 N, float3 V, float3 baseColor, float roughness, float metallic)
{
    // return specularIBL(N, V, baseColor, metallic, roughness*roughness, 1024);
    // roughness = pow(roughness, 2.2);
    float NoV = saturate(dot(N, V));
    float3 R = 2 * dot(V, N) * N - V; // reflection
    float2 brdf = gBrdfMap.Sample(gsamLinear, float2(roughness, 1.0 - NoV)).rg;
    float lod = DecodeLOD(roughness);
    float3 radiance = gEnvMap.SampleLevel(gsamLinear, normalize(R), lod).rgb;
    float3 f0 = lerp(float3(0.04, 0.04, 0.04), baseColor, metallic);
    return radiance * (f0 * brdf.x + brdf.y);
}

float3 ApproximateDiffuseIBL(float3 N, float3 diffuseColor)
{
    float mipLevels = 4;
    // hack
    return diffuseColor * gEnvMap.SampleLevel(gsamLinear, normalize(N), mipLevels*0.5).rgb;
}

float3 AmbientEnvLight(float3 N, float3 V, float3 baseColor, float metallic, float roughness)
{
    return ApproximateDiffuseIBL(N, (1.0-metallic)*baseColor) + ApproximateSpecularIBL(N, V, baseColor, roughness, metallic);
}

float4 PS(VertexOut pin): SV_TARGET
{   
    float4 sample0 = diffuseMetallic.Load(int3(pin.pos.x, pin.pos.y, 0)).rgba;
    float4 sample1 = normalRoughness.Load(int3(pin.pos.x, pin.pos.y, 0)).rgba;
    float4 sample2 = worldPosX.Load(int3(pin.pos.x, pin.pos.y, 0)).rgba;
    float4 sample3 = viewPosY.Load(int3(pin.pos.x, pin.pos.y, 0)).rgba;
    float3 baseColor = sample0.rgb;
    float metallic = sample0.a;
    float3 normal = normalize(sample1.rgb);
    float roughness = sample1.a;
    float3 viewDir = normalize(sample3.rgb);
    float ao = sample2.a;
    float3 worldPos = sample2.rgb;
    float emissive = sample3.a;
    float3 L = normalize(_lightDir);
    float inten = _lightInten;

    if(baseColor.r==0.0 && baseColor.g==0.0 && baseColor.b==0.0)
        return float4(0.0, 0.0, 0.0, 1.0);

    float3 testColor = baseColor * saturate(dot(normal, L));
    // return float4(testColor, 1.0);

    float3 color = BRDF_Faliment(normal, viewDir, L, baseColor, metallic, roughness) * inten * ao * saturate(dot(normal, L));

    if(_mainLight){
        float4x4 vp = mul(_JProj, _SMView);
        float4 clipPos = mul(vp, float4(worldPos, 1.0));
        vp = mul(_SMProj, _SMView);
        float shadowZ = mul(vp, float4(worldPos, 1.0)).z;
        float factor = GetShadowBlur(clipPos, shadowZ);
        color = color * (1.0 - factor);

        color = color + AmbientEnvLight(normal, viewDir, baseColor, metallic, roughness) * ao;

        color = color + emissive;
    }

    return float4(color, 1.0);
}