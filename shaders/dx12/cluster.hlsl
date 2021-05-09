cbuffer PassUniform : register(b0)
{
	float4x4 _View; // debug 时候这个不能变
	float4x4 _Proj;
};

cbuffer ObjectUniform : register(b1)
{
	float4x4 _model;
    uint _id;
};

struct VertexIn
{
	float3 vertex: POSITION;
};

struct VertexOut
{
	float4 pos: POSITION;
};

struct GeoOut 
{
    float4 pos: SV_POSITION;
    float temp: TEXCOORD;
    uint rtArrayIndex : SV_RenderTargetArrayIndex;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	float4x4 mvp = mul(mul(_model, _View), _Proj);

	vout.pos = mul(float4(vin.vertex, 1.0f), mvp);
    return vout;
}

[maxvertexcount(3)]
void GS(triangle VertexOut gin[3], inout TriangleStream<GeoOut> stream)
{
    GeoOut gout;
    gout.rtArrayIndex = _id;
    for(int i = 0; i < 3; i++){
        gout.pos = gin[i].pos;
        gout.temp = (gin[i].pos.z / gin[i].pos.w)*0.5 + 0.5;
        stream.Append(gout);
    }
}

float2 PS(GeoOut pin) : SV_TARGET
{
    // TODO: more exact max depth & min depth, for now I'm tired...
    return float2(1.0 - pin.temp, pin.temp);
}