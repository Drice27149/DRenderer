Texture2D gNormalMap: register(t0);
Texture2D gDiffuseMap: register(t1);
Texture2D gMetallicMap: register(t2);
Texture2D gEmissiveMap: register(t3);
Texture2D gHeightMap: register(t4);
Texture2D gShadowMap: register(t5);

SamplerState gsamLinear: register(s0);

cbuffer RealPass : register(b0)
{
	float4x4 _View;
	float4x4 _Proj;
	float4x4 _SMView;
	float4x4 _SMProj;
	float4x4 _JProj;
	float4x4 _lastView;
	float4x4 _lastProj;
	float3 _CamPos;
};

cbuffer RealObject: register(b1)
{
	float4x4 _model;
	uint _id;
    uint _mask;
    float _metallic;
    float _roughness;
	float _cx;
	float _cy;
	float _cz;
};

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
	float2 uv: TEXCOORD0;
	float3 T: TEXCOORD1;
	float3 N: TEXCOORD2;
	float3 worldPos: TEXCOORD4;
    float3 viewPos: TEXCOORD5;
	float4 clipPos: TEXCOORD6;
	float4 lastClipPos: TEXCOORD7;
};

struct PixelOut 
{
    float4 diffuseMetallic: SV_TARGET0;
    float4 normalRoughness: SV_TARGET1;
    float4 worldPosX: SV_TARGET2;
    float4 viewPosY: SV_TARGET3;
	float2 velocity: SV_TARGET4;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	float4x4 mvp = mul(mul(_Proj, _View), _model);
	vout.pos = mul(mvp, float4(vin.vertex, 1.0f));

	vout.worldPos = mul(_model, float4(vin.vertex, 1.0f)).rgb;
    float4x4 vp = mul(_View, _model);
    vout.viewPos = mul(vp, float4(vin.vertex, 1.0f)).rgb;

	mvp = mul(mul(_Proj, _View), _model); // no jitter
	vout.clipPos = mul(mvp, float4(vin.vertex, 1.0f));
	float4x4 lastMvp = mul(mul(_lastProj, _lastView), _model);
	vout.lastClipPos = mul(lastMvp, float4(vin.vertex, 1.0f));

	vout.uv = vin.texcoord;
	// in worldspace
    vout.T = mul(_model, float4(vin.tangent, 0.0)).rgb;
	vout.N = mul(_model, float4(vin.normal, 0.0)).rgb;
    return vout;
}

// normal mapping
float3 tangentToWorldNormal(float3 normal, float3 N, float3 T)
{
	N = normalize(N);
    T = normalize(T - dot(T, N)*(N));
    float3 B = cross(N, T);
    float3x3 TBN = transpose(float3x3(T, B, N));
	return normalize(mul(TBN, normal));
}

// bump mapping

float3 GetLightDir()
{
	return normalize(float3(-1.0, 1.0, 1.0));
}

float SampleShadowMap(float4 clipPos)
{
	clipPos.xyz /= clipPos.w;
	float x = clipPos.x * 0.5 + 0.5;
	float y = (-clipPos.y) * 0.5 + 0.5;
	if(x>=0.0 && x<=1.0 && y>=0.0 && y<=1.0)
		return gShadowMap.Sample(gsamLinear, float2(x,y));
	else
		return 1.0;
}

float GetShadowBlur(float4 clipPos)
{
	clipPos.xyz /= clipPos.w;
	float x = clipPos.x * 0.5 + 0.5;
	float y = (-clipPos.y) * 0.5 + 0.5;
	if(x>=0.0 && x<=1.0 && y>=0.0 && y<=1.0){
		float depth = gShadowMap.Sample(gsamLinear, float2(x,y));
		if(clipPos.z < depth + 0.005) 
			return 0.0;
		else 
			return 1.0;
	}
	else
		return 0.0;
}

float2 PixelMotionVector(VertexOut pin)
{
	float width = 1080;
	float height = 780;
	float4 clipPos = pin.clipPos;
	clipPos = clipPos / clipPos.w;
	float2 now = float2((clipPos.x*0.5+0.5)*width, (1.0 - (clipPos.y*0.5+0.5))*height);
	float4 lastClip = pin.lastClipPos;
	lastClip = lastClip / lastClip.w;
	float2 last = float2((lastClip.x*0.5+0.5)*width, (1.0 - (lastClip.y*0.5+0.5))*height);
	float2 offset = last - now;
	return offset;
}

PixelOut PS(VertexOut pin)
{
	float2 uv = pin.uv;
	float3 baseColor = gDiffuseMap.Sample(gsamLinear, uv).rgb;
	baseColor = pow(baseColor, 2.2);
	float ao = 1.0;
	float roughness = gMetallicMap.Sample(gsamLinear, uv).g;
	float emissive = gEmissiveMap.Sample(gsamLinear, uv).r;
	float metallic = gMetallicMap.Sample(gsamLinear, uv).b;
	float3 normal = pin.N;
	float height = gHeightMap.Sample(gsamLinear, uv).r;

	if(false){
		ao = gMetallicMap.Sample(gsamLinear, uv).r;
	}
	if(_mask & 1){
		normal = gNormalMap.Sample(gsamLinear, uv).rgb;
		normal = normal*2.0 - 1.0;
		normal = tangentToWorldNormal(normal, pin.N, pin.T);
	}
	if(!(_mask & 2)){
		baseColor = float3(1.0, 1.0, 1.0);
	}
	if(baseColor.r > 0.9 && baseColor.g < 0.1 && baseColor.b < 0.1)
		baseColor = float3(1.0, 1.0, 1.0);
	if(!(_mask & 64)){
		roughness = _roughness;
		metallic = _metallic;
	}
	if(_mask & 32){
		//normal = normalize(float3(height, height, height));
	}

    PixelOut pixOut;
    pixOut.diffuseMetallic = float4(baseColor, metallic);
    pixOut.normalRoughness = float4(normal, roughness);
    pixOut.worldPosX = float4(pin.worldPos, ao);
    pixOut.viewPosY = float4(normalize(_CamPos-pin.worldPos), emissive);
	pixOut.velocity = PixelMotionVector(pin);

	return pixOut;
}