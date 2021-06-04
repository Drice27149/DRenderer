Texture2D gPixMap: register(t0);

cbuffer testInfo : register(b0)
{
    float _color;
};

struct VertexIn
{
	float3 vertex: POSITION;
};

struct VertexOut
{
	float4 pos: SV_POSITION;
    float pixColor: TEXCOORD0;
};

VertexOut VS(VertexIn vin, uint id: SV_VertexID)
{
    VertexOut vout;
	if(id == 0 || id==5){
        vout.pos = float4(-1.0, 1.0, 0.0, 1.0);
    }
    else if(id==1){
        vout.pos = float4(1.0, 1.0, 0.0, 1.0);
    }
    else if(id==2 || id==3){
        vout.pos = float4(1.0, -1.0, 0.0, 1.0);
    }
    else{
        vout.pos = float4(-1.0, -1.0, 0.0, 1.0);
    }
    vout.pixColor = _color;
    return vout;
}

float4 PS(VertexOut pin): SV_TARGET
{   
    return gPixMap.Load(int3(pin.pos.x, pin.pos.y, 0));
    // return float4(pin.pixColor, pin.pixColor, pin.pixColor, 1.0);
}