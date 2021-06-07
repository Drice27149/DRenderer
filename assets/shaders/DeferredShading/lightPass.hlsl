#include "BRDF.hlsl"

Texture2D diffuseMetallic: register(t0);
Texture2D normalRoughness: register(t1);
Texture2D worldPosX: register(t2);
Texture2D viewPosY: register(t3);

SamplerState gsamLinear: register(s0);

cbuffer LightDesc : register(b0)
{
    float3 _lightDir;
    float _lightInten;
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
    float3 viewDir = sample3.rgb;
    float ao = sample2.a;
    float emissive = sample3.a;
    float3 L = normalize(_lightDir);
    float inten = 2.0; // _lightIten;

    if(baseColor.r==0.0 && baseColor.g==0.0 && baseColor.b==0.0)
        return float4(0.0, 0.0, 0.0, 1.0);

    float3 testColor = baseColor * saturate(dot(normal, L));
    // return float4(testColor, 1.0);

    float3 color = BRDF_Faliment(normal, viewDir, L, baseColor, metallic, roughness) * inten * ao * saturate(dot(normal, L));
    color = color + emissive;

    return float4(color, 1.0);
}