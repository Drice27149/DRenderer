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

RWStructuredBuffer<Offset> headTable: register(u0);
RWStructuredBuffer<Node> nodeTable: register(u1);
// RWStructuredBuffer<Light> lightTable: register(u2);
Texture2DArray<float2> depthTable: register(t0);
StructuredBuffer<Light> lightTable: register(t1);

[numthreads(16, 8, 1)]
void CS(int3 id : SV_DispatchThreadID)
{
    Offset noff;
    noff.offset = headTable.IncrementCounter();
    headTable[noff.offset-1] = noff;
    // 伪代码
    // 获得裁剪空间下的最小和最大深度
    // for 该 tile 下的所有 cluster
    // 如果在最小深度和最大深度之间, 把光源注册
    // 注册流程如下
    // node 表新加一个单元, id 指向当前光源
    // next 指向这个 cluster 的 head
    // 把这个 cluster 的 head 改为 node 表的这个新单元

}
