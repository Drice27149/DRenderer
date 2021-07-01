
float3 GetGridPos(float3 worldPos, float gridSize, float gridCnt)
{
    float rate = 1.0 / gridSize;
    float3 res = float3(worldPos.x * rate, worldPos.y * rate, worldPos.z * rate);
    res = res + float3(0.5, 0.5, 0.5);
    return res;
}


float3 GetRadiance(float3 worldPos, float3 normal, Texture3D voxelMip, SamplerState linerSampler, float gridSize, float gridCnt)
{
    int len = gridCnt;
    float maxLevel = 0.0;
    while(len!=0){
        len /= 2;
        maxLevel += 1.0;
    }
    float3 res = 0.0;
    float occ = 0.0;
    float sumStep = 1.0;
    float aspect = 45.0 / 180.0 * 3.1415926;
    float rate = tan(aspect);
    // one shot for now
    float3 dir = normalize(normal);
    float3 cc = float3(0.0, 0.0, 0.0);
    float sampleCnt = 0.0;
    for(int it = 0; it < 128; it++){
        float sampleLen = tan(aspect) * sumStep;
        float lod = maxLevel - log2(gridSize / sampleLen);
        float3 center = sumStep + worldPos + dir * sampleLen;
        center = GetGridPos(center, gridSize, gridCnt);
        if(center.x>=0.0 && center.x <= gridCnt && center.y>=0.0 && center.y <= gridCnt && center.z >= 0.0 && center.z <= gridCnt){
            float4 value = voxelMip.SampleLevel(linerSampler, center, lod).rgba;
            float3 addColor = value.rgb * (1.0 - occ); 
            //addColor = float3(1.0, 1.0, 1.0);
            cc = cc * occ + (1.0 - occ) * addColor;
            // occ = occ + (1.0 - occ) * value.a;
            sampleCnt = sampleCnt + 1.0;
            // step forward
            sumStep = sumStep + 2 * sampleLen;
        }
        else{
            break;
        }
    }
    return cc / sampleCnt;
}

float3 GetAO(float3 worldPos, float3 normal, Texture3D voxelMip, SamplerState linerSampler, float gridSize, float gridCnt)
{
    int len = gridCnt;
    float maxLevel = 0.0;
    while(len!=0){
        len /= 2;
        maxLevel += 1.0;
    }
    float3 res = 0.0;
    float occ = 0.0;
    float sumStep = 10.0;
    float aspect = 45.0 / 180.0 * 3.1415926;
    float rate = tan(aspect);
    // one shot for now
    float3 dir = normalize(normal);
    float3 cc = float3(0.0, 0.0, 0.0);
    float sampleCnt = 0.0;
    int totalStep = 1;
    for(int it = 0; it < totalStep; it++){
        float sampleLen = tan(aspect) * sumStep;
        float lod = maxLevel - log2(gridSize / sampleLen);
        float3 center = sumStep + worldPos + dir * sampleLen;
        center = GetGridPos(center, gridSize, gridCnt);
        if(center.x>=0.0 && center.x <= gridCnt && center.y>=0.0 && center.y <= gridCnt && center.z >= 0.0 && center.z <= gridCnt){
            float4 value = voxelMip.SampleLevel(linerSampler, center, lod).rgba;
            float3 addColor = value.rgb * (1.0 - occ); 
            //addColor = float3(1.0, 1.0, 1.0);
            cc = cc * occ + (1.0 - occ) * addColor;
            // occ = occ + (1.0 - occ) * value.a;
            occ = occ + value.a;
            sampleCnt = sampleCnt + 1.0;
            // step forward
            sumStep = sumStep + 2 * sampleLen;
        }
        else{
            break;
        }
    }
    return (1.0 - occ) / totalStep;
    return cc / sampleCnt;
}