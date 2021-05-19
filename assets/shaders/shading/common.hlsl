Texture2D gShadowMap: register(t0);
Texture2D gNormalMap: register(t1);
Texture2D gDiffuseMap: register(t2);
Texture2D gMetallicMap: register(t3);
Texture2D gEmissiveMap: register(t4);
TextureCube gCubeMap: register(t6);

SamplerState gsamLinear: register(s0);

cbuffer RealPass : register(b0)
{
	float4x4 _View;
	float4x4 _Proj;
	float4x4 _SMView;
	float4x4 _SMProj;
	float3 _CamPos;
};

cbuffer RealObject: register(b1)
{
	float4x4 _model;
	uint _id;
    uint _mask;
    float _metallic;
    float _roughness;
};

cbuffer SceneInfo: register(b3)
{
    float _MainLightPosX;
    float _MainLightPosY;
    float _MainLightPosZ;
    float _MainLightDirX;
    float _MainLightDirY;
    float _MainLightDirZ;
    float _lightIntensity;
    float _envIntensity;
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
	float3 worldPos: TEXCOORD4;
};