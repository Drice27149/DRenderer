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
    // z = 2.0 * z - 1.0;
    // float depth = 2.0 * n * f / (f + n - z * (f - n));
    float depth = z * (f-n);
    return depth;
}

[numthreads(16, 8, 1)]
void CS(int3 groupID: SV_GROUPID, int3 threadID : SV_GROUPTHREADID)
{
    // α����
    // ��òü��ռ��µ���С��������
    // for �� tile �µ����� cluster
    // �������С��Ⱥ�������֮��, �ѹ�Դע��
    // ע����������
    // node ���¼�һ����Ԫ, id ָ��ǰ��Դ
    // next ָ����� cluster �� head
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
        // ���������� cluster ����ע�������Դ
        // ��ǰ cluster �� id
        float cMinD = near + (float)z * stepLen;
        float cMaxD = cMinD + stepLen;
        if(minDepth > cMaxD || maxDepth < cMinD){}
        else{
            uint cID = index * clusterZ + z;
            // ���� node �� id
            uint lastOff;
            uint newOff = nodeTable.IncrementCounter();
            // ���� head
            InterlockedExchange(headTable[cID], newOff, lastOff);
            // �� node
            Node newNode;
            newNode.next = lastOff;
            newNode.lightID = lightID;
            nodeTable[newOff] = newNode;
        }
    }
}
