cbuffer RealPass : register(b0)
{
	float4x4 _View; 
	float4x4 _Proj;
};

cbuffer ObjectCB: register(b1)
{
	float4x4 _model;
}

struct VertexIn
{
	float3 vertex: POSITION;
};

struct VertexOut
{
	float4 pos: SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	float4x4 mvp = mul(mul(_model, _View), _Proj);
	vout.pos = mul(float4(vin.vertex, 1.0f), mvp);

    return vout;
}

float4 PS(VertexOut pin): SV_TARGET
{
    return float4(1.0, 1.0, 1.0, 1.0);
}