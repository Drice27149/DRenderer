TextureCube gCubeMap: register(t0);
SamplerState gsamLinear: register(s0);

float radicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 hammersley2D(uint i, uint N)
{
    return float2(float(i)/float(N), radicalInverse_VdC(i));
}

float3x3 computeTangetSpace(float3 normal)
{
    float3 up = abs(normal.y) > 0.999 ? float3(0.0, 0.0, 1.0) : float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, normal));
    up = cross(normal, right);
    return transpose(float3x3(right, up, normal));
}

float3 hemisphereSampleGGX(float u1, float u2, float roughness )
{
    float phi = 2.0*3.1415926*u1;
    float cos_theta = sqrt((1.0-u2)/(u2*(roughness*roughness-1)+1));
    float sin_theta = sqrt(1-cos_theta*cos_theta);
    // spherical to cartesian conversion
    float3 dir;
    dir.x = cos(phi)*sin_theta;
    dir.y = sin(phi)*sin_theta;
    dir.z = cos_theta;
    return dir;
}

float3 PrefilterEnvMap(float Roughness, float3 R, uint NumSamples)
{
    float3 N = R;
    float3 V = R;
    float3x3 tangentSpace = computeTangetSpace(N);
    float3 PrefilteredColor = 0;
    float3 TotalWeight = 0;
    for( uint i = 0; i < NumSamples; i++ ){
        float2 rand_value = hammersley2D(i, NumSamples);
        float3 H = normalize(mul(tangentSpace, hemisphereSampleGGX(rand_value[0], rand_value[1], Roughness)));
        float3 L = 2 * dot( V, H ) * H - V;
        float NoL = saturate( dot( N, L ) );
        if( NoL > 0 ){
            PrefilteredColor += gCubeMap.Sample(gsamLinear, normalize(L)).rgb * NoL;
            TotalWeight += NoL;
        }
    }
    return PrefilteredColor/TotalWeight;
}
