cbuffer RealPass : register(b0)
{
	float4x4 _View;
	float4x4 _Proj;
	float4x4 _SMView;
	float4x4 _SMProj;
	float4x4 _JProj;
	float4x4 _lastView;
	float4x4 _lastProj;
	float3 _CamPos;
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
	float4 pos: SV_POSITION;
    float shadowZ: TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	float4x4 mvp = mul(mul(_Proj, _SMView), _model);
	vout.pos = mul(mvp, float4(vin.vertex, 1.0f));
    mvp = mul(mul(_SMProj, _SMView), _model);
    vout.shadowZ = mul(mvp, float4(vin.vertex, 1.0f)).z;

    return vout;
}

float4 PS(VertexOut pin): SV_TARGET
{
    return float4(pin.shadowZ, pin.shadowZ, pin.shadowZ, 1.0);
}