Texture2D gHistoryMap: register(t0);
Texture2D gPixMap: register(t1);
Texture2D gVelocity: register(t2);
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

bool isValid(int2 uv)
{
	int width = 860;
	int height = 720;
    if(uv.x>=0 && uv.x<width && uv.y>=0 && uv.y<height)
        return true;
    else    
        return false;
}

float max3(float3 u)
{
    return max(max(u.x, u.y), u.z);
}

float3 clipToBox(float3 boxmin, float3 boxmax, float3 c, float3 h) {
    float epsilon = 0.0001;
    float3 r = c - h;
    float3 ir = 1.0 / (epsilon + r.rgb);
    float3 rmax = (boxmax - h.rgb) * ir;
    float3 rmin = (boxmin - h.rgb) * ir;
    float3 imin = min(rmax, rmin);
    return h + r * saturate(max3(imin));
}

float3 getClipHistory(int2 uv, float3 history)
{
    float3 s[9];
    int id = 0;
    for(int i = -1; i <= 1; i++){
        for(int j = -1; j <= 1; j++){
            s[id++] = gPixMap.Load(int3(i+uv.x, j+uv.y, 0)).rgb;
        }
    }
    
    float3 boxmin = min(s[4], min(min(s[1], s[3]), min(s[5], s[7])));
    float3 boxmax = max(s[4], max(max(s[1], s[3]), max(s[5], s[7])));
    float3 box9min = min(boxmin, min(min(s[0], s[2]), min(s[6], s[8])));
    float3 box9max = max(boxmax, max(max(s[0], s[2]), max(s[6], s[8])));
    // round the corners of the 3x3 box
    boxmin = (boxmin + box9min) * 0.5;
    boxmax = (boxmax + box9max) * 0.5;

    return clipToBox(boxmin, boxmax, s[4], history);
}

float4 PS(VertexOut pin, float4 pos: SV_POSITION): SV_TARGET
{   
    int u = pin.pos.x;
    int v = pin.pos.y;
    float2 duv = gVelocity.Load(int3(u, v, 0)).rg;
    int du = duv.x;
    int dv = duv.y;
    // return float4(du, dv, 0.0, 1.0);
    float4 now = gPixMap.Load(int3(u, v, 0));
    if(isValid(int2(u+du, v+dv))){
        int2 uv = int2(u+du, v+dv);
        float4 history = gHistoryMap.Load(int3(uv, 0));
        float3 clipH = getClipHistory(uv, history.rgb);
        return lerp(float4(clipH, 1.0), now, _taaAlpha);
    }
    else
        return now;
}