Texture2D _colorBuffer: register(t0);
Texture2D _historyBuffer: register(t1);
Texture2D _velocity: register(t2);

SamplerState gsamLinear: register(s0);

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
    return vout;
}

float max3(float3 u)
{
    return max(max(u.x, u.y), u.z);
}

float luma(const float3 color) {
    // return max3(color.rgb);
    return 0.299*color.r + 0.587*color.g + 0.114*color.b;
}

float3 RGB_YCoCg(const float3 c) {
    float Y  = dot(c.rgb, float3( 1, 2,  1) * 0.25);
    float Co = dot(c.rgb, float3( 2, 0, -2) * 0.25);
    float Cg = dot(c.rgb, float3(-1, 2, -1) * 0.25);
    return float3(Y, Co, Cg);
}

float3 YCoCg_RGB(const float3 c) {
    float Y  = c.x;
    float Co = c.y;
    float Cg = c.z;
    float r = Y + Co - Cg;
    float g = Y + Cg;
    float b = Y - Co - Cg;
    return float3(r, g, b);
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
            s[id++] = _colorBuffer.Load(int3(i+uv.x, j+uv.y, 0)).rgb;
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

float3 toneMapping(float3 color, float _adapted_lum)
{
    color = ACESToneMapping(color, _adapted_lum);
    float gamma = 2.2;
    color = pow(color, 1.0/gamma);
    return color;
}

float4 PS(VertexOut pin): SV_TARGET
{   
    // flicker: tonemap before blend
    float taaAlpha = 0.05;
    float toneLum = 0.5;

    int u = pin.pos.x;
    int v = pin.pos.y;
    float2 duv = _velocity.Load(int3(u, v, 0)).rg;
    int du = duv.x;
    int dv = duv.y;
    du = 0;
    dv = 0;
    float3 now = _colorBuffer.Load(int3(u, v, 0)).rgb;

    //return float4(now, 1.0);

    int2 uv = int2(u+du, v+dv);
    float3 history = _historyBuffer.Load(int3(uv, 0)).rgb;

    history = getClipHistory(uv, history.rgb);

    float lumaNow = luma(now);
    float lumaHis = luma(history);

    float diff = abs(lumaNow - lumaHis) / (0.001 + max(lumaNow, lumaHis));
    // return float4(lumaNow, lumaHis, 0.0, 1.0);
    // if(diff > 0.95) 
    //     return float4(now, 1.0);

    // tonemapping for handling HDR
    now.rgb *= 1.0 / (1.0 + luma(now));
    history.rgb  *= 1.0 / (1.0 + luma(history));

    // combine history and current frame
    float3 result = lerp(history, now, taaAlpha);

    // untonemap result
    result.rgb *= 1.0 / (1.0 - luma(result.rgb));

    result.r = max(0.0, result.r);
    result.g = max(0.0, result.g);
    result.b = max(0.0, result.b);

    return float4(result, 1.0);
}