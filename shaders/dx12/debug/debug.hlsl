struct Node {
    uint id;
    uint next;
};

struct Offset {
    uint offset;
};

struct Light {
    uint id;
};

RWStructuredBuffer<Offset> headTable: register(u0);
RWStructuredBuffer<Node> nodeTable: register(u1);
RWTexture2D<float4> outTable: register(u2);

[numthreads(16, 1, 1)]
void CS(int3 id : SV_DispatchThreadID)
{
    int2 pos = int2(1, id.x);
    outTable[pos] = float4(0.5, 0.5, 0.5, 1.0);
}
