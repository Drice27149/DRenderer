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
    vout.T = normalize(vin.tangent);
	vout.N = normalize(vin.normal);

    return vout;
}

float3 tangentToWorldNormal(float3 n, float3 T, float3 N)
{
	float3 B = normalize(cross(T,N));
	float3x3 TBN = float3x3(T,B,N);
	return mul(TBN, n);
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 baseColor = gDiffuseMap.Sample(gsamLinear, pin.uv);
	float3 normal = gNormalMap.Sample(gsamLinear, pin.uv);
	normal = normal*2.0 + 1.0;
	normal = tangentToWorldNormal(normal, pin.T, pin.N);

	baseColor = float4(normal, 1.0);

    return baseColor;
}