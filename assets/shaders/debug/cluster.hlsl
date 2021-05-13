cbuffer RealPass : register(b0)
{
	float4x4 _View; 
	float4x4 _Proj;
};

cbuffer ObjectCB: register(b1)
{
	float4x4 _model;
}

cbuffer ClusterUniform : register(b2)
{
    uint clusterX;
    uint clusterY;
    uint clusterZ;
    float near;
    float far;
};

struct VertexIn
{
	float3 vertex: POSITION;
	float3 normal: NORMAL;
    float2 texcoord: TEXCOORD;
	float3 tangent: TANGENT;
	float3 bitangent: COLOR;
	uint x: TEXCOORD1;
	uint y: TEXCOORD2;
	uint z: TEXCOORD3;
};

struct VertexOut
{
	float4 pos: POSITION;
    uint x: TEXCOORD0;
	uint y: TEXCOORD1;
	uint z: TEXCOORD2;
};

struct GeoOut 
{
    float4 pos: SV_POSITION;
};

struct Node {
    uint id;
    uint next;
};

RWStructuredBuffer<uint> headTable: register(u0);
RWStructuredBuffer<Node> nodeTable: register(u1);

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	float4x4 mvp = mul(mul(_model, _View), _Proj);
	vout.pos = mul(float4(vin.vertex, 1.0f), mvp);

	vout.x = vin.x;
	vout.y = vin.y;
	vout.z = vin.z;

    return vout;
}

int GetClusterID(int x,int y,int z)
{
	if(x>=0 && x<clusterX && y>=0 && y<clusterY && z>=0 && z<clusterZ){
		int index = y*clusterX + x;
		return index*clusterZ + z;
	}
	else
		return -1;
}

bool haveLight(int index)
{
	uint result = headTable[index];
	if(result != 0) 
		return true;
	else 
		return false;
}

int3 GetMaskPos(int3 pos, int mask)
{
	if(mask&1) pos.x = pos.x - 1;
	if(mask&2) pos.y = pos.y - 1;
	if(mask&4) pos.z = pos.z - 1;
	return pos;
}

[maxvertexcount(3)]
void GS(line VertexOut gin[2], inout LineStream<GeoOut> stream)
{
	// x: [0, clusterX]
	// y: [0, clusterY]
	// z: [0, clusterZ]
    bool findLight = false;
	for(int i = 0; i < 8; i++){
		int3 ipos = GetMaskPos(int3(gin[0].x, gin[0].y, gin[0].z), i);
		for(int j = 0; j < 8; j++){
			int3 jpos = GetMaskPos(int3(gin[1].x, gin[1].y, gin[1].z), j);
			if(ipos.x == jpos.x && ipos.y == jpos.y && ipos.z == jpos.z){
				int index = GetClusterID(ipos.x, ipos.y, ipos.z);
				if(index != -1){
					// findLight = true;
					// if(index == 25 || index == 26) findLight = true;
					if(haveLight(index))
						findLight = true;
				}
			}
		}
	}
	if(findLight){
		GeoOut gout;
		gout.pos = gin[0].pos;
		stream.Append(gout);
		gout.pos = gin[1].pos;
		stream.Append(gout);
	}
}

float4 PS(GeoOut pin): SV_TARGET
{
    return float4(1.0, 1.0, 1.0, 1.0);
}