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
            int3 index = int3(x,y,z);
            uint r = voxelGridR[index];
            uint g = voxelGridG[index];
            uint b = voxelGridB[index];
            uint a = voxelGridA[index];
            float3 color;
            float u = r;
            color.x = u / 1000.0;
            u = g;
            color.y = u / 1000.0;
            u = b;
            color.z = u / 1000.0;
            float ra = a;
            if(a>0)
                color = color / ra;
            res = float4(color, min(ra, 1.0));
        }
        else{
            res = fromVoxel[index];
        }
        sum = sum + res / 8.0;
    }
    toVoxel[int3(x,y,z)] = sum;
    //toVoxel[int3(x,y,z)] = float4(1.0, 1.0, 1.0, 1.0); 
} 