Texture2D gPixMap: register(t0);

cbuffer PassID: register(b0)
{
    uint _id;
};

struct VertexIn
{
	float3 vertex: POSITION;
};

struct VertexOut
{
	float4 pos: SV_POSITION;
};

VertexOut VS(VertexIn vin, uint id: SV_VertexID)
{
    VertexOut vout;
	if(id == 0 || id==5)
        vout.pos = float4(-1.0, 1.0, 0.0, 1.0);
    else if(id==1)
        vout.pos = float4(1.0, 1.0, 0.0, 1.0);
    else if(id==2 || id==3)
        vout.pos = float4(1.0, -1.0, 0.0, 1.0);
    else
        vout.pos = float4(-1.0, -1.0, 0.0, 1.0);
    return vout;
}

float4 PS(VertexOut pin, float4 pos: SV_POSITION): SV_TARGET
{   
    if(_id%2==0)
        return float4(1.0, 0.0, 0.0, 1.0);
    else    
        return float4(0.0, 1.0, 0.0, 1.0);
}