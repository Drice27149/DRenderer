TextureCube gCubeMap: register(t3);
SamplerState gsamLinear: register(s0);

cbuffer RealPass : register(b0)
{
	float4x4 View;
	float4x4 Proj;
};

cbuffer ObjectCB: register(b1)
{
	float4x4 model;
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
    float3 uv: TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	float4x4 rotation = View;
	for(int i = 0; i < 3; i++) rotation[3][i] = 0.0;

	float4x4 mvp = mul(mul(model, rotation), Proj);

	vout.pos = mul(float4(vin.vertex, 1.0f), mvp);
    vout.uv = vin.vertex;
    return vout;
}

float4 PS(VertexOut pin): SV_TARGET
{
    return gCubeMap.Sample(gsamLinear, pin.uv);
}