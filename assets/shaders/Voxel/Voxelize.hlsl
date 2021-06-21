cbuffer PassConstant : register(b0)
{
    int _width;
    int _height;
    int _depth;
    int _sizeX;
    int _sizeY;
    int _sizeZ;
    float4x4 _orthoProj;
};

cbuffer RealObject: register(b1)
{
	float4x4 _model;
	uint _id;
    uint _mask;
    float _metallic;
    float _roughness;
	float _cx;
	float _cy;
	float _cz;
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
	float3 pos: POSITION;
    float shadowZ: TEXCOORD;
};

struct GeoOut {
    float4 pos: SV_POSITION;
    uint face: TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	// float4x4 mvp = mul(mul(_JProj, _SMView), _model);
	// vout.pos = mul(mvp, float4(vin.vertex, 1.0f));
    // mvp = mul(mul(_SMProj, _SMView), _model);
    // vout.shadowZ = mul(mvp, float4(vin.vertex, 1.0f)).z;
    vout.pos = vin.vertex;

    return vout;
}

[maxvertexcount(3)]
void GS(triangle VertexOut gin[3], inout TriangleStream<GeoOut> stream)
{
    GeoOut gout;
    float3 pos[3];
    for(int i = 0; i < 3; i++){
        pos[i] = gin[i].pos;
    }

    for(int i = 0; i < 3; i++){
        gout.pos = float4(0.0, 0.0, 0.0, 1.0);
        gout.face = 0;
        stream.Append(gout);
    }
}

void PS(GeoOut pin)
{

}