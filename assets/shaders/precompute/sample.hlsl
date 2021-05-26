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

float3 hemisphereSample(float u1, float u2)
{
    float phi = u1 * 2.0 * 3.1415926;
    float r = sqrt(u2);
    return float3( r*cos(phi), r*sin(phi), sqrt(1-u2));
}

float3 diffuseIBL(float3 normal, uint NUM_SAMPLES)
{
    float3 irradiance = float3(0.0, 0.0, 0.0);
    float3x3 tangentSpace = computeTangetSpace(normal);
    for(int i=0; i < NUM_SAMPLES; ++i)
    {
        float2 rand_value = hammersley2D(i, NUM_SAMPLES);
        float3 sample_dir = mul(tangentSpace, hemisphereSample(rand_value[0], rand_value[1]));

        irradiance += gCubeMap.Sample(gsamLinear, normalize(sample_dir)).rgb;
    }
    return irradiance * (1.0/float(NUM_SAMPLES));
}

float3 specularIBL(float3 normal, uint NUM_SAMPLES)
{
    
}
