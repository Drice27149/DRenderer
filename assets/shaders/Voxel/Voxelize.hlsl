cbuffer PassConstant : register(b0)
{
    int _width;
    int _height;
    int _depth;
    int _sizeX;
    int _sizeY;
    int _sizeZ;
    float4x4 _orthoProj;
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
	float3 pos: POSITION;
    float shadowZ: TEXCOORD;
};

struct GeoOut {
    float4 pos: SV_POSITION;
    uint face: TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	// float4x4 mvp = mul(mul(_JProj, _SMView), _model);
	// vout.pos = mul(mvp, float4(vin.vertex, 1.0f));
    // mvp = mul(mul(_SMProj, _SMView), _model);
    // vout.shadowZ = mul(mvp, float4(vin.vertex, 1.0f)).z;
    float4 pos = mul(_model, float4(vin.vertex, 1.0f));
    vout.pos = pos.xyz;

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
        stream.Append(gout);
    }
}

RWTexture3D<uint4> voxelGrid: register(u0);

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
    int4 origin = voxelGrid[int3(x,y,z)];
    origin.x += 1;
    voxelGrid[int3(x,y,z)] = origin;
    //InterlockedAdd(voxelGrid[int3(x,y,z)].x, addCount);
}
// debug pix: 110, 128