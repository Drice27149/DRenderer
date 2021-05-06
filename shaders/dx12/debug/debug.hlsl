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
RWStructuredBuffer<Light> lightTable: register(u2);
RWTexture2D<float4> outTable: register(u3);

[numthreads(8, 1, 1)]
void CS(int3 id : SV_DispatchThreadID)
{
    int2 pos = int2(id.x, 0);
    float4 cur = outTable[pos];
    cur.x = cur.x + 0.01;
    outTable[pos] = cur;
}
