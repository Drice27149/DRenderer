Texture2D gShadowMap: register(t0);
Texture2D gNormalMap: register(t1);
Texture2D gDiffuseMap: register(t2);

SamplerState gsamLinear: register(s0);

cbuffer RealPass : register(b0)
{
	float4x4 _View;
	float4x4 _Proj;
	float4x4 _SMView;
	float4x4 _SMProj;
};

cbuffer RealObject: register(b1)
{
	float4x4 _model;
};

struct VertexIn
{
	float3 vertex: POSITION;
	float3 normal: NORMAL;
    float2 texcoord: TEXCOORD;
	float3 tangent: TANGENT;
	float3 bitangent: TANGENT1;
};

struct VertexOut
{
	float4 pos: SV_POSITION;
	float2 uv: TEXCOORD;
	float3 T: TEXCOORD1;
	float3 N: TEXCOORD2;
	float4 clipPos: TEXCOORD3;
};