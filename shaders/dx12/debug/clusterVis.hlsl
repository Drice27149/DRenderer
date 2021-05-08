cbuffer RealPass : register(b0)
{
	float4x4 _View;
	float4x4 _Proj;
};

cbuffer ObjectCB: register(b1)
{
	float4x4 _model;
}

cbuffer ClusterUniform : register(b2)
{
    uint clusterX;
    uint clusterY;
    uint clusterZ;
    float near;
    float far;
};

struct VertexIn
{
	float3 vertex: POSITION;
	float3 normal: NORMAL;
    float2 texcoord: TEXCOORD;
	float3 tangent: TANGENT;
	float3 bitangent: COLOR;
	uint row: TEXCOORD1;
	uint col: TEXCOORD2;
};

struct VertexOut
{
	float4 pos: POSITION;
    uint row: TEXCOORD0;
	uint col: TEXCOORD1;
};

struct GeoOut 
{
    float4 pos: SV_POSITION;
};

struct Node {
    uint id;
    uint next;
};

RWStructuredBuffer<uint> headTable: register(u0);
RWStructuredBuffer<Node> nodeTable: register(u1);

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	float4x4 mvp = mul(mul(_model, _View), _Proj);
	vout.pos = mul(float4(vin.vertex, 1.0f), mvp);
	vout.row = vin.row;
	vout.col = vin.col;

    return vout;
}

[maxvertexcount(3)]
void GS(line VertexOut gin[2], inout LineStream<GeoOut> stream)
{
    GeoOut gout;
	for(int i = 0; i < 2; i++){
		gout.pos = gin[i].pos;
		stream.Append(gout);
	}	
}

float4 PS(GeoOut pin): SV_TARGET
{
    return float4(1.0, 1.0, 1.0, 1.0);
}