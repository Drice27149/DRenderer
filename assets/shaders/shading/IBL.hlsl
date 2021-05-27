#include "common.hlsl"

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
    return float3x3(right, up, normal);
}

float3 hemisphereSample(float u1, float u2)
{
    float phi = u1 * 2.0 * 3.1415926;
    float r = sqrt(u2);
    return float3( r*cos(phi), r*sin(phi), sqrt(1-u2));
}

float3 diffuseIBL(float3 normal, float3 diffuseColor, uint NUM_SAMPLES)
{
    float3 irradiance = float3(0.0, 0.0, 0.0);
    float3x3 tangentSpace = computeTangetSpace(normal);
    // for(int i=0; i < NUM_SAMPLES; ++i)
    // {
    //     float2 rand_value = hammersley2D(i, NUM_SAMPLES);
    //     float3 sample_dir = mul(hemisphereSample(rand_value[0], rand_value[1]), tangentSpace);

    //     irradiance += gCubeMap.Sample(gsamLinear, normalize(sample_dir)).rgb;
    // }
    // 由于是重要性采样, pdf和NoL已经被消掉了, 所以 baseColor 不用除以 pi, 也不用乘上NoL
    float3 color = diffuseColor * irradiance * (1.0/float(NUM_SAMPLES));
    return color;
}

float3 hemisphereSampleGGX(float u1, float u2, float roughness )
{
    float phi = 2.0*3.1415926*u1;
    float cos_theta = sqrt((1.0-u2)/(u2*(roughness*roughness-1)+1));
    float sin_theta = sqrt(1-cos_theta*cos_theta);
    // spherical to cartesian conversion
    vec3 dir;
    dir.x = cos(phi)*sin_theta;
    dir.y = sin(phi)*sin_theta;
    dir.z = cos_theta;
    return dir;
}

float3 specularIBL(float3 N, float3 V, float3 baseColor, float metallic, float roughness, uint NUM_SAMPLES)
{
    float3 irradiance = float3(0.0, 0.0, 0.0);
    float3x3 tangentSpace = computeTangetSpace(N);
    // UE4: 0.08
    float3 f0 = lerp(float3(0.04, 0.04, 0.04), baseColor, metallic);
    float a = roughness * roughness;

    for(int i = 0; i < NUM_SAMPLES; i++)
    {
        float2 rand_value = hammersley2D(i, NUM_SAMPLES);
        float3 H = normalize(mul(hemisphereSampleGGX(rand_value[0], rand_value[1], roughness), tangentSpace));
        float3 L = normalize(2 * dot( V, H ) * H - V);
        float NoV = saturate( dot( N, V ) );
        float NoL = saturate( dot( N, L ) );
        float NoH = saturate( dot( N, H ) );
        float VoH = saturate( dot( V, H ) );
        float LoH = saturate( dot( L, H ) );
        if( NoL > 0 )
        {
            float3 SampleColor = gEnvMap.SampleLevel(gsamLinear, normalize(L), 0.0).rgb;
            float3 F = F_Schlick(LoH, f0);
            float Vis = V_SmithGGXCorrelated(NoV, NoL, a);
            // Incident light = SampleColor * NoL
            // Microfacet specular = D*F*V
            // pdf = D * NoH / (4 * VoH)
            irradiance += 4.0 * VoH * SampleColor * NoL * F * Vis  / NoH;
        }
    }

    return irradiance * (1.0/float(NUM_SAMPLES));
}

float3 ApproximateDiffuseIBL(float3 N, float3 diffuseColor)
{
    float mipLevels = 4;
    // hack
    return diffuseColor * gEnvMap.SampleLevel(gsamLinear, normalize(N), mipLevels*0.5).rgb;
}

float3 ApproximateSpecularIBL(float3 N, float3 V, float3 baseColor, float roughness, float metallic)
{
    return specularIBL(N, V, baseColor, metallic, roughness, 1024);
    // this is shit
    float mipLevels = 4;
    float NoV = dot(N, V);
    if(NoV <= 0.0)
        return 0.0;
    float2 brdf = gBrdfMap.Sample(gsamLinear, float2(roughness, NoV)).rg;
    float3 color = gEnvMap.SampleLevel(gsamLinear, normalize(N), 1.0).rgb;
    float3 f0 = lerp(float3(0.04, 0.04, 0.04), baseColor, metallic);
    return color * (f0 * brdf.x + brdf.y);
}

float3 AmbientIBL(float3 N, float3 V, float3 baseColor, float roughness, float metallic)
{
	// uint NUM_SAMPLE = 512;
	// // float3 fd = diffuseIBL(N, (1.0-metallic)*baseColor, NUM_SAMPLE);
    // float3 fd = PreComputeDiffuse(N, (1.0-metallic)*baseColor);
	// float3 fr = specularIBL(N, V, baseColor, metallic, roughness, NUM_SAMPLE);
	// return fd + fr;
    return float3(0.0, 0.0, 0.0);
}

float3 ApproximateIBL(float3 N, float3 V, float3 baseColor, float roughness, float metallic)
{
    return ApproximateDiffuseIBL(N, (1.0-metallic)*baseColor) + ApproximateSpecularIBL(N, V, baseColor, roughness, metallic);
}

