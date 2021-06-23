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
    float4 pos: POSITION1;
	float3 worldPos: POSITION2;
	uint4 voxel: POSITION0;
};

struct GeoOut {
    float4 pos: SV_POSITION;
	float3 normal: POSITION0;
};

RWTexture3D<uint4> voxelGrid: register(u0);

int3 GetOffsetByID(int id)
{
	int unit = _width;
	int3 res;
	res.x = id % unit;
	res.y = (id / unit) % unit;
	res.z = (id/(unit*unit)) % unit;
	return res;
}

VertexOut VS(VertexIn vin, uint id: SV_INSTANCEID)
{
	VertexOut vout;
	float4x4 model;
	int step = _sizeX / _width;
	int3 index = GetOffsetByID(id);
	vout.voxel = voxelGrid[index];
	int3 offset = (index - int3(_width/2, _width/2, _width/2)) * step;
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			if(i==j){
				if(i==3) model[i][j] = 1.0;
				else model[i][j] = step*0.5;
			}
			else
				model[i][j] = 0.0;
		}
	}
	model[0][3] += offset.x;
	model[1][3] += offset.y;
	model[2][3] += offset.z;
	float4x4 mvp = mul(mul(_Proj, _View), model);
	vout.pos = mul(mvp, float4(vin.vertex, 1.0f));
	vout.worldPos = mul(model, float4(vin.vertex, 1.0f)).xyz;
    return vout;
}

[maxvertexcount(3)]
void GS(triangle VertexOut gin[3], inout TriangleStream<GeoOut> stream)
{
	GeoOut gout;
	if(gin[0].voxel.x != 0){
		float3 normal = normalize(cross(gin[1].worldPos-gin[0].worldPos, gin[2].worldPos-gin[0].worldPos));
		for(int i = 0; i < 3; i++){
			gout.pos = gin[i].pos;
			gout.normal = normal;
			stream.Append(gout);
		}
	}
}

float4 PS(GeoOut pin): SV_TARGET
{
	float3 lightDir = normalize(float3(-1.0, 5.0, 5.0));
	float c = saturate(dot(lightDir, pin.normal));
    return float4(c, c, c, 1.0);
}