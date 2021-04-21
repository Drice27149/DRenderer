Texture2D gDiffuseMap: register(t0);
Texture2D gNormalMap: register(t1);
SamplerState gsamLinear: register(s0);

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj; 
};

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
	float2 uv: TEXCOORD;
	float3 T: TEXCOORD1;
	float3 N: TEXCOORD2;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	vout.pos = mul(float4(vin.vertex, 1.0f), gWorldViewProj);
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
	return float3(1.0, 1.0, 1.0);
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 baseColor = gDiffuseMap.Sample(gsamLinear, pin.uv);
	float3 normal = gNormalMap.Sample(gsamLinear, pin.uv).rgb;
	normal = normal*2.0 - 1.0;
	normal = tangentToWorldNormal(normal, pin.N, pin.T);
	float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
	float lightRate = dot(lightDir, normal);

	baseColor =	float4((baseColor.rgb) * lightRate, 1.0);

    return baseColor;
}