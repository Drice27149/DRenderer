cbuffer PassUniform : register(b0)
{
	float4x4 _View;
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
    float depth: POSITION1;
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
    float4x4 mv = mul(_model, _View);
    float4 viewPos = mul(float4(vin.vertex, 1.0f), mv);

	vout.pos = mul(float4(vin.vertex, 1.0f), mvp);
    vout.depth = (-1.0) * (viewPos.z / 19.0);
    return vout;
}

[maxvertexcount(3)]
void GS(triangle VertexOut gin[3], inout TriangleStream<GeoOut> stream)
{
    GeoOut gout;
    gout.rtArrayIndex = _id;
    for(int i = 0; i < 3; i++){
        gout.pos = gin[i].pos;
        gout.temp = gin[i].depth;
        stream.Append(gout);
    }
}

bool closeTo(float src, float dest)
{
    return abs(src - dest) < 20.0;
}

float2 PS(GeoOut pin) : SV_TARGET
{
    float dtemp = pin.temp;
    float x = 1.0 - dtemp;
    float y = dtemp;
    // TODO: more exact max depth & min depth, for now I'm tired...
    if(x>=0.99 || x <= 0.01 || y>=0.99 || y<=0.01)
        return float2(0.0, 0.0);
    return float2(x, y);
    // if(y >= 0.6 && y<=0.7)
    //     return float2(x, y);
    // else 
    //     return float2(0.0, 0.0);
}