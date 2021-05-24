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

float4 PS(VertexOut pin, float4 pos: SV_POSITION) : SV_TARGET
{
    float3 orgC = gPixMap.Load(int3(pos.x, pos.y, 0)).rgb;
    if(Luminance(orgC) >= 1.0) // magic number
        return float4(orgC, 1.0);
    uint x = pos.x;
    uint y = pos.y;
    uint width;
    uint height;
    gPixMap.GetDimensions(width, height);
    float3 sum = float3(0.0, 0.0, 0.0);
    for (int i = -5; i <= 5; i++) {
        for (int j = -5; j <= 5; j++) {
            int r = x + i;
            int c = y + j;
            if (r > 0 && r < width && c>0 && c < height) {
                float3 color = gPixMap.Load(int3(r, c, 0)).rgb;
                // sum += color;
                float dis = abs(i) + abs(j) + 0.25;
                sum += color / dis / 40.0;
            }
        }
    }
    return float4(sum, 1.0);
}