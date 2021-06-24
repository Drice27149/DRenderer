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
	float3 color: POSITION1;
};

RWTexture3D<uint> voxelGridR: register(u0);
RWTexture3D<uint> voxelGridG: register(u1);
RWTexture3D<uint> voxelGridB: register(u2);
RWTexture3D<uint> voxelGridA: register(u3);

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
	vout.voxel = uint4(voxelGridR[index], voxelGridG[index], voxelGridB[index], voxelGridA[index]);
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

float3 UnpackInt2Float(int4 u)
{
    float3 res;
	float v = u.x;
	res.x = v/1000.0;
	v = u.y;
	res.y = v/1000.0;
	v = u.w;
	res.z = v/1000.0;
	float num = u.w;
	res = res / num;
	return res;
}

[maxvertexcount(3)]
void GS(triangle VertexOut gin[3], inout TriangleStream<GeoOut> stream)
{
	GeoOut gout;
	if(gin[0].voxel.w != 0){
		float3 normal = normalize(cross(gin[1].worldPos-gin[0].worldPos, gin[2].worldPos-gin[0].worldPos));
		for(int i = 0; i < 3; i++){
			gout.pos = gin[i].pos;
			gout.normal = normal;
			gout.color = UnpackInt2Float(gin[i].voxel);
			stream.Append(gout);
		}
	}
}

float4 PS(GeoOut pin): SV_TARGET
{
    return float4(pin.color, 1.0);
}