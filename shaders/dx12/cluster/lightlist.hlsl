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

[numthreads(16, 1, 1)]
void CS(int3 id : SV_DispatchThreadID)
{
    Offset noff;
    noff.offset = headTable.IncrementCounter();
    headTable[noff.offset-1] = noff;
}
