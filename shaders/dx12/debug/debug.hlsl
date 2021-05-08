struct Node {
    uint id;
    uint next;
};

struct Offset {
    uint offset;
};

struct Light {
    uint id;
    float3 pos;
    float radiance;
};

cbuffer ClusterUniform : register(b0)
{
    uint clusterX;
    uint clusterY;
    uint clusterZ;
    float near;
    float far;
};

RWStructuredBuffer<uint> headTable: register(u0);
RWStructuredBuffer<Node> nodeTable: register(u1);
RWTexture2D<float4> outTable: register(u2);
// RWStructuredBuffer<Light> lightTable: register(u2);
Texture2DArray<float2> depthTable: register(t0);
StructuredBuffer<Light> lightTable: register(t1);

uint GetLightCount(uint cID)
{
    uint count = 0;
    uint offset = headTable[cID];
    while(offset != 0)
    {
        count = count + 1;
        offset = nodeTable[offset].next;
    }
    return count;
}

[numthreads(16, 8, 1)]
void CS(int3 groupID: SV_GROUPID, int3 threadID : SV_GROUPTHREADID)
{
    uint index = threadID.y * clusterX + threadID.x;
    uint count = 0;
    for(int z = 0; z < clusterZ; z++){
        uint cID = index * clusterZ + z;
        count = count + GetLightCount(cID);
    }
    float value = (float)count / (float)10.0;
    outTable[int2(threadID.x, threadID.y)] = float4(value, 0.0, 0.0, 0.0);
}
