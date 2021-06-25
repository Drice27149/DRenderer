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

[numthreads(16, 8, 1)]
void CS(int3 groupID: SV_GROUPID, int3 threadID : SV_GROUPTHREADID)
{

}