Texture2D gShadowMap: register(t2);
SamplerState gsamLinear: register(s0);

cbuffer cbPerObject : register(b0)
{
	float4x4 viewProj; 
};

cbuffer ObjectCB: register(b1)
{
	float4x4 model;
}

cbuffer PassCB: register(b2)
{
	float4x4 View;
	float4x4 Proj;
}

struct VertexIn
{
	float3 vertex: POSITION;
	float3 normal: NORMAL;
    float2 texcoord: TEXCOORD;
	float3 tangent: TANGENT;
	float3 bitangent: COLOR;
};

struct VertexOut
{
	float4 pos: SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	float4x4 mvp = mul(mul(model, View), Proj);

	vout.pos = mul(float4(vin.vertex, 1.0f), mvp);
    return vout;
}

void PS(VertexOut pin)
{
}