cbuffer MipCostant : register(b0)
{
    uint level;
};

RWTexture3D<uint> voxelGridR: register(u0);
RWTexture3D<uint> voxelGridG: register(u1);
RWTexture3D<uint> voxelGridB: register(u2);
RWTexture3D<uint> voxelGridA: register(u3);
RWTexture3D<float4> fromVoxel: register(u4);
RWTexture3D<float4> toVoxel: register(u5);                   

[numthreads(1, 1, 1)]
void CS(int3 groupID: SV_GROUPID, int3 threadID : SV_GROUPTHREADID)
{
    int x = groupID.x;
    int y = groupID.y;
    int z = groupID.z;
    float4 sum = float4(0.0, 0.0, 0.0, 0.0);
    for(int i = 0; i < 8; i++){
        int mask = i;
        int dx = mask%2; mask /= 2;
        int dy = mask%2; mask /= 2;
        int dz = mask%2; mask /= 2;
        int3 index = int3(2*x+dx, 2*y+dy, 2*z+dz);
        float4 res = float4(0.0, 0.0, 0.0, 0.0);
        if(level==0){
            int r = voxelGridR[index];
            int g = voxelGridG[index];
            int b = voxelGridB[index];
            int a = voxelGridA[index];
            float rf = r;
            float rg = g;
            float rb = b;
            float ra = a;
            float3 color = float3(rf, rg, rb) / ra;
            res = float4(color, max(ra, 1.0));
        }
        else{
            res = fromVoxel[index];
        }
        sum = sum + res / 8.0;
    }
    toVoxel[int3(x,y,z)] = sum;
} 