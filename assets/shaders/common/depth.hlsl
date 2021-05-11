cbuffer PassUniform : register(b0)
{
	float4x4 _view;
	float4x4 _proj;
};

cbuffer ObjectUniform: register(b1)
{
	float4x4 _model;
};

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
	
	// Transform to homogeneous clip space.
	float4x4 mvp = mul(mul(_model, _view), _proj);

	vout.pos = mul(float4(vin.vertex, 1.0f), mvp);
    return vout;
}

void PS(VertexOut pin)
{
}