TextureCube gCubeMap: register(t0);
SamplerState gsamLinear: register(s0);

cbuffer RealPass : register(b0)
{
	float4x4 View;
	float4x4 Proj;
	float4x4 _SMView;
	float4x4 _SMProj;
	float4x4 _JProj;
	float4x4 _lastView;
	float4x4 _lastProj;
	float3 _CamPos;
};

cbuffer ObjectCB: register(b1)
{
	float4x4 model;
}

struct VertexIn
{
	float3 vertex: POSITION;
	float3 normal: NORMAL;
    float2 texcoord: TEXCOORD;
	float3 tangent: TANGENT;
	float3 bitangent: TANGENT1;
};

struct VertexOut
{
	float4 pos: SV_POSITION;
    float3 uv: TEXCOORD;
	float4 clipPos: TEXCOORD1;
	float4 lastClipPos: TEXCOORD2;
};

struct PixOut
{
	float4 color: SV_TARGET0;
	float2 velocity: SV_TARGET1;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	float4x4 temp;
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			if(i==3 || j==3) 
				temp[i][j] = 0.0;
			else
				temp[i][j] = View[i][j];
		}
	}
	temp[3][3] = 1.0;

	float4x4 lastTemp;
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			if(i==3 || j==3) 
				lastTemp[i][j] = 0.0;
			else
				lastTemp[i][j] = _lastView[i][j];
		}
	}
	lastTemp[3][3] = 1.0;

	float4x4 mvp = mul(_JProj, temp);
	vout.pos = mul(mvp, float4(vin.vertex, 1.0f)).xyww;
    vout.uv = vin.vertex;
	mvp = mul(Proj, temp);
	vout.clipPos = mul(mvp, float4(vin.vertex, 1.0f)).xyww;
	mvp = mul(_lastProj, lastTemp);
	vout.lastClipPos = mul(mvp, float4(vin.vertex, 1.0f)).xyww;

    return vout;
}

float2 PixelVelocity(VertexOut pin)
{
	float width = 860;
	float height = 720;
	float2 now = pin.clipPos.xy / pin.clipPos.w;
	float2 last = pin.lastClipPos.xy / pin.lastClipPos.w;
	now = float2((now.x*0.5+0.5)*width, (0.5-now.y*0.5)*height);
	last = float2((last.x*0.5+0.5)*width, (0.5-last.y*0.5)*height);
	float diff = last-now;
	return float2(0.0, 0.0);
	return diff;
}

PixOut PS(VertexOut pin): SV_TARGET
{
	PixOut pixOut;
	pixOut.color = gCubeMap.Sample(gsamLinear, pin.uv);
	pixOut.velocity = PixelVelocity(pin);
	return pixOut;
}