Texture2D gPixMap: register(t0);
SamplerState gsamLinear: register(s0);

cbuffer SceneInfo: register(b0)
{
    float _MainLightPosX;
    float _MainLightPosY;
    float _MainLightPosZ;
    float _MainLightDirX;
    float _MainLightDirY;
    float _MainLightDirZ;
    float _lightIntensity;
    float _envIntensity;
    int _taa;
    float _taaAlpha;
    float _adapted_lum;
    float _threshold;
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

float Luminance(float3 c)
{
    return 0.299*c.r + 0.587*c.g + 0.114*c.b;
}

float4 PS(VertexOut pin, float4 pos: SV_POSITION): SV_TARGET
{   
    int x = pos.x;
    int y = pos.y;
    float3 color = gPixMap.Load(int3(x, y, 0)).rgb;
    if(Luminance(color) >= _threshold)
        return float4(color, 1.0);
    else 
        return float4(0.0, 0.0, 0.0, 1.0);
}