RWTexture2D<float4> inPix: register(u0);
RWTexture2D<float4> outPix: register(u1);

[numthreads(16, 8, 1)]
void CS(int3 groupID: SV_GROUPID, int3 threadID : SV_GROUPTHREADID)
{
}