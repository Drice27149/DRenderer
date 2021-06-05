Texture2D gShadowMap: register(t0);
Texture2D gNormalMap: register(t1);
Texture2D gDiffuseMap: register(t2);
Texture2D gMetallicMap: register(t3);
Texture2D gEmissiveMap: register(t4);
// TextureCube gEnvMap: register(t6);
// Texture2D gBrdfMap: register(t7);

SamplerState gsamLinear: register(s0);

cbuffer RealPass : register(b0)
{
	float4x4 _View;
	float4x4 _Proj;
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
	float3 worldPos: TEXCOORD4;
};

struct PixelOut 
{
    float4 diffuseMetallic: SV_TARGET0;
    float4 normalRoughness: SV_TARGET1;
    float4 worldPosAO: SV_TARGET2;
    float2 velocity: SV_TARGET3;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	float4x4 mvp = mul(mul(_Proj, _View), _model);
	vout.pos = mul(mvp, float4(vin.vertex, 1.0f));
	vout.worldPos = mul(_model, float4(vin.vertex, 1.0f)).rgb;

	vout.uv = vin.texcoord;
	// in worldspace
    vout.T = mul(_model, float4(vin.tangent, 0.0)).rgb;
	vout.N = mul(_model, float4(vin.normal, 0.0)).rgb;
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

PixelOut PS(VertexOut pin)
{
	PixelOut pixOut;
	return pixOut;
}