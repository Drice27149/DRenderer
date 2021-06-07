Texture2D _colorBuffer: register(t0);

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
    float3 color = _colorBuffer.Load(int3(pin.pos.x, pin.pos.y, 0));
    float lum = 0.5;
    return float4(toneMapping(color, lum), 1.0);
}