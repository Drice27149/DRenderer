struct Node {
    uint lightID;
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
// RWStructuredBuffer<Light> lightTable: register(u2);
Texture2DArray<float2> depthTable: register(t0);
StructuredBuffer<Light> lightTable: register(t1);

float GetlinerDepth(float z, float n, float f)
{
    z = 2.0 * z - 1.0;
    float depth = 2.0 * n * f / (f + n - z * (f - n));
    return depth;
}

[numthreads(16, 8, 1)]
void CS(int3 groupID: SV_GROUPID, int3 threadID : SV_GROUPTHREADID)
{
    // 伪代码
    // 获得裁剪空间下的最小和最大深度
    // for 该 tile 下的所有 cluster
    // 如果在最小深度和最大深度之间, 把光源注册
    // 注册流程如下
    // node 表新加一个单元, id 指向当前光源
    // next 指向这个 cluster 的 head
    uint index = threadID.y * clusterX + threadID.x;
    uint lightID = groupID.x;
    // Offset pre = headTable[index];
    // Offset cur;
    // cur.offset = nodeTable.IncrementCounter();
    // headTable[index] = cur;
    uint x = threadID.x;
    uint y = threadID.y;
    float2 result = depthTable.Load(int4(x, y, lightID, 0));
    float minDepth = 1.0 - result.x;
    float maxDepth = result.y;
    if(result.x == 0.0f && result.y == 0.0f) return ;
    minDepth = GetlinerDepth(minDepth, near, far);
    maxDepth = GetlinerDepth(maxDepth, near, far);
    float stepLen = (far - near) / (float)(clusterZ);
    for(uint z = 0; z < clusterZ; z++){
        // 现在是所有 cluster 都会注册这个光源
        // 当前 cluster 的 id
        float cMinD = near + (float)z * stepLen;
        float cMaxD = cMinD + stepLen;
        if(minDepth > cMaxD || maxDepth < cMinD){}
        else{
            uint cID = index * clusterZ + z;
            // 新增 node 的 id
            uint lastOff;
            uint newOff = nodeTable.IncrementCounter();
            // 交换 head
            InterlockedExchange(headTable[cID], newOff, lastOff);
            // 新 node
            Node newNode;
            newNode.next = lastOff;
            newNode.lightID = lightID;
            nodeTable[newOff] = newNode;
        }
    }
}
