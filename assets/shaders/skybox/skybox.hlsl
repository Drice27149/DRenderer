TextureCube gCubeMap: register(t0);
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
	float3 bitangent: TANGENT1;
};

struct VertexOut
{
	float4 pos: SV_POSITION;
    float3 uv: TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	float4x4 temp;
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			if(i==3 || j==3) 
				temp[i][j] = 0.0;
			else
				temp[i][j] = View[i][j];
		}
	}
	temp[3][3] = 1.0;

	float4x4 mvp = mul(Proj, temp);

	vout.pos = mul(mvp, float4(vin.vertex, 1.0f)).xyww;
    vout.uv = vin.vertex;
    return vout;
}

float4 PS(VertexOut pin): SV_TARGET
{
    return gCubeMap.Sample(gsamLinear, pin.uv);
}