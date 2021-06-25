#include "../DeferredShading/BRDF.hlsl"

Texture2D gNormalMap: register(t0);
Texture2D gDiffuseMap: register(t1);
Texture2D gMetallicMap: register(t2);
Texture2D gEmissiveMap: register(t3);
Texture2D gShadowMap: register(t4);

SamplerState gsamLinear: register(s0);

cbuffer VoxelConstant : register(b0)
{
    int _width;
    int _height;
    int _depth;
    int _sizeX;
    int _sizeY;
    int _sizeZ;
    float4x4 _orthoProj;
};

cbuffer PassConstant: register(b1)
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

cbuffer RealObject: register(b2)
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
	float3 pos: POSITION;
	float2 uv: TEXCOORD0;
	float3 T: TEXCOORD1;
	float3 N: TEXCOORD2;
	float3 worldPos: TEXCOORD4;
};

struct GeoOut {
    float4 pos: SV_POSITION;
    uint face: TEXCOORD;
    float2 uv: TEXCOORD3;
	float3 T: TEXCOORD1;
	float3 N: TEXCOORD2;
	float3 worldPos: TEXCOORD4;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

    float4 pos = mul(_model, float4(vin.vertex, 1.0f));
    vout.pos = pos.xyz;

    vout.uv = vin.texcoord;
	// in worldspace
    vout.T = mul(_model, float4(vin.tangent, 0.0)).rgb;
	vout.N = mul(_model, float4(vin.normal, 0.0)).rgb;
    vout.worldPos = mul(_model, float4(vin.vertex, 1.0f)).rgb;

    return vout;
}

[maxvertexcount(3)]
void GS(triangle VertexOut gin[3], inout TriangleStream<GeoOut> stream)
{
    float3 normal = normalize(cross(gin[1].pos-gin[0].pos, gin[2].pos-gin[1].pos));
    float xValue = abs(normal.x);
    float yValue = abs(normal.y);
    float zValue = abs(normal.z);

    GeoOut gout;
    gout.face = -1;
    float4 pos[3];
    float len = _sizeX;
    float cnt = _width;
    float rate = 2.0/len;

    if(xValue > yValue && xValue > zValue){
        // width = y, height = z
        gout.face = 0;
        for(int i = 0; i < 3; i++){
            pos[i].x = gin[i].pos.y * rate;
            pos[i].y = gin[i].pos.z * rate;
            pos[i].z = gin[i].pos.x/len + 0.5;
            pos[i].w = 1.0;
        }
    }
    else if(yValue > xValue && yValue > zValue){
        // width = x, height = z
        gout.face = 1;
        for(int i = 0; i < 3; i++){
            pos[i].x = gin[i].pos.x * rate;
            pos[i].y = gin[i].pos.z * rate;
            pos[i].z = gin[i].pos.y/len + 0.5;
            pos[i].w = 1.0;
        }
    }
    else{
        // width = x, height = y
        gout.face = 2;
        for(int i = 0; i < 3; i++){
            pos[i].x = gin[i].pos.x * rate;
            pos[i].y = gin[i].pos.y * rate;
            pos[i].z = gin[i].pos.z/len + 0.5;
            pos[i].w = 1.0;
        }
    }

    for(int i = 0; i < 3; i++){
        gout.pos = pos[i];
        gout.uv = gin[i].uv;
        gout.T = gin[i].T;
        gout.N = gin[i].N;
        gout.worldPos = gin[i].worldPos;
        stream.Append(gout);
    }
}

float3 tangentToWorldNormal(float3 normal, float3 N, float3 T)
{
	N = normalize(N);
    T = normalize(T - dot(T, N)*(N));
    float3 B = cross(N, T);
    float3x3 TBN = transpose(float3x3(T, B, N));
	return normalize(mul(TBN, normal));
}

float GetVisibility(float4 clipPos, float z)
{
	// clipPos.xyz /= clipPos.w;
	float x = clipPos.x/clipPos.w * 0.5 + 0.5;
	float y = (-clipPos.y/clipPos.w) * 0.5 + 0.5;
	if(x>=0.0 && x<=1.0 && y>=0.0 && y<=1.0){
		float depth = gShadowMap.Sample(gsamLinear, float2(x,y)).r;
		if(z < depth + 0.02) 
			return 1.0;
		else 
			return 0.0;
	}
	else
		return 1.0;
}

float3 DirectLightColor(GeoOut pin)
{
	float2 uv = pin.uv;
	float3 baseColor = gDiffuseMap.Sample(gsamLinear, uv).rgb;
	baseColor = pow(baseColor, 2.2);
	float ao = 1.0;
	float roughness = gMetallicMap.Sample(gsamLinear, uv).g;
	float emissive = gEmissiveMap.Sample(gsamLinear, uv).r;
	float metallic = gMetallicMap.Sample(gsamLinear, uv).b;
	float3 normal = pin.N;
    float3 worldPos = pin.worldPos;
    float3 viewDir = normalize(_CamPos-pin.worldPos);
    float3 L = normalize(float3(-1.0, 1.0, 1.0));
    float inten = 1.0;

    if(_mask & 1){
		normal = gNormalMap.Sample(gsamLinear, uv).rgb;
		normal = normal*2.0 - 1.0;
		normal = tangentToWorldNormal(normal, pin.N, pin.T);
	}
	if(!(_mask & 2)){
		baseColor = float3(1.0, 1.0, 1.0);
	}
	if(!(_mask & 64)){
		roughness = _roughness;
		metallic = _metallic;
	}

    float3 color = BRDF_Faliment(normal, viewDir, L, baseColor, metallic, roughness) * inten * ao * saturate(dot(normal, L));
    // shadow mapping
    float4x4 vp = mul(_Proj, _SMView);
    float4 clipPos = mul(vp, float4(worldPos, 1.0));
    vp = mul(_SMProj, _SMView);
    float shadowZ = mul(vp, float4(worldPos, 1.0)).z;
    float vis = GetVisibility(clipPos, shadowZ);

    //color = color * vis;

    return color;
}

uint3 PackFloat2Int(float3 u)
{
    uint3 res;
    float v = u.x*1000.0;
    res.x = v;
    v = u.y*1000.0;
    res.y = v;
    v = u.z*1000.0;
    res.z = v;
    return res;
}

RWTexture3D<uint> voxelGridR: register(u0);
RWTexture3D<uint> voxelGridG: register(u1);
RWTexture3D<uint> voxelGridB: register(u2);
RWTexture3D<uint> voxelGridA: register(u3);

void PS(GeoOut pin)
{
    int len = _width;
    int x = -1;
    int y = -1;
    int z = -1;
    if(pin.face==0){
        // y, z, x
        x = (pin.pos.z) * len;
        y = pin.pos.x;
        z = len - pin.pos.y;
    }
    else if(pin.face==1){
        // x, z, y
        y = (pin.pos.z) * len;
        x = pin.pos.x;
        z = len - pin.pos.y;
    }
    else{
        // x, y, z
        z = (pin.pos.z) * len;
        x = pin.pos.x;
        y = len - pin.pos.y;
    }

    float3 color = DirectLightColor(pin);
    uint4 packColor = int4(PackFloat2Int(color), 1);
    int3 index = int3(x,y,z);
    InterlockedAdd(voxelGridR[index], packColor.r);
    InterlockedAdd(voxelGridG[index], packColor.g);
    InterlockedAdd(voxelGridB[index], packColor.b);
    InterlockedAdd(voxelGridA[index], packColor.a);
    //InterlockedAdd(voxelGrid[int3(x,y,z)].x, addCount);
}
// debug pix: 110, 128