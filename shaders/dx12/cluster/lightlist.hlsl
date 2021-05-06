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

[numthreads(16, 1, 1)]
void CS(int3 id : SV_DispatchThreadID)
{
    Offset noff;
    noff.offset = headTable.IncrementCounter();
    headTable[id.x] = noff;
}
