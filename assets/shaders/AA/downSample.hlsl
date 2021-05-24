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

float Luminance(float3 c)
{
    return 0.299*c.r + 0.587*c.g + 0.114*c.b;
}

float4 PS(VertexOut pin, float4 pos: SV_POSITION): SV_TARGET
{   
    uint dx;
    uint dy;
    gPixMap.GetDimensions(dx, dy);
    int x = pos.x*2;
    int y = pos.y*2;
    float3 sum = float3(0.0, 0.0, 0.0);
    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 2; j++){
            float3 color = gPixMap.Load(int3(x+i, y+j, 0)).rgb;
            if(Luminance(color) >= _threshold || dx!=860){
                sum += color;
            }
        }
    }
    sum = sum / 4.0;
    return float4(sum, 1.0);
}