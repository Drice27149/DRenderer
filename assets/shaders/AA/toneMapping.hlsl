Texture2D gPixMap: register(t0);
Texture2D bloomBlur: register(t1);

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
    int _bloom;
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

float3 ACESToneMapping(float3 color, float adapted_lum)
{
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;

	color *= adapted_lum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}

float4 PS(VertexOut pin, float4 pos: SV_POSITION): SV_TARGET
{   
    int x = pos.x;
    int y = pos.y;
    float3 color = gPixMap.Load(int3(x, y, 0)).rgb;
    if(_bloom){
        color += bloomBlur.Load(int3(x, y, 0)).rgb;
    }
    color = ACESToneMapping(color, _adapted_lum);
    float gamma = 2.2;
    color = pow(color, 1.0/gamma);
    return float4(color, 1.0);
}