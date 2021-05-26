#include "sample.hlsl"

cbuffer PassID: register(b0)
{
    uint _id;
    float _roughness;
};

struct VertexIn
{
	float3 vertex: POSITION;
};

struct VertexOut
{
	float4 pos: SV_POSITION;
    float2 uv: TEXCOORD;
};

VertexOut VS(VertexIn vin, uint id: SV_VertexID)
{
    VertexOut vout;
	if(id==0 || id==5){
        vout.uv = float2(-1.0, 1.0);
        vout.pos = float4(-1.0, 1.0, 0.0, 1.0);
    }
    else if(id==1){
        vout.uv = float2(1.0, 1.0);
        vout.pos = float4(1.0, 1.0, 0.0, 1.0);
    }
    else if(id==2 || id==3){
        vout.uv = float2(1.0, -1.0);
        vout.pos = float4(1.0, -1.0, 0.0, 1.0);
    }
    else{
        vout.uv = float2(-1.0, -1.0);
        vout.pos = float4(-1.0, -1.0, 0.0, 1.0);
    }   
    return vout;
}

// SV_POSITION: screen space coordinate
float4 PS(VertexOut pin) : SV_TARGET
{
    float2 uv = pin.uv;
    float3 dir;
    // left
    if(_id==0)
        dir = float3(1.0, uv.y, -uv.x);
    // right
    else if(_id==1)
        dir = float3(-1.0, uv.yx);
    // top
    else if(_id==2)
        dir = float3(uv.x, 1.0, -uv.y);
    // down
    else if(_id==3)
        dir = float3(uv.x, -1.0, uv.y);
    // front
    else if(_id==4)
        dir = float3(uv, 1.0);
    // back
    else
        dir = float3(-uv.x, uv.y, -1.0);
    // @TODO: prefilter mip-map to speed up convergence
    float3 color = diffuseIBL(normalize(dir), 4096);
    return float4(color, 1.0);
}
