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
    return transpose(float3x3(right, up, normal));
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
        float3 H = normalize(mul(tangentSpace, hemisphereSampleGGX(rand_value[0], rand_value[1], roughness)));
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

float2 hammersley(uint i, uint N)
{
    // Radical inverse based on http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
    uint bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10;
    return float2(float(i) / float(N), rdi);
}

// From the filament docs. Geometric Shadowing function
// https://google.github.io/filament/Filament.html#toc4.4.2
float G_Smith(float NoV, float NoL, float roughness)
{
    float k = (roughness * roughness) / 2.0;
    float GGXL = NoL / (NoL * (1.0 - k) + k);
    float GGXV = NoV / (NoV * (1.0 - k) + k);
    return GGXL * GGXV;
}

// Based on Karis 2014
vec3 importanceSampleGGX(vec2 Xi, float roughness, vec3 N)
{
    float a = roughness * roughness;
    // Sample in spherical coordinates
    float Phi = 2.0 * PI * Xi.x;
    float CosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float SinTheta = sqrt(1.0 - CosTheta * CosTheta);
    // Construct tangent space vector
    vec3 H;
    H.x = SinTheta * cos(Phi);
    H.y = SinTheta * sin(Phi);
    H.z = CosTheta;

    // Tangent to world space
    vec3 UpVector = abs(N.z) < 0.999 ? vec3(0., 0., 1.0) : vec3(1.0, 0., 0.);
    vec3 TangentX = normalize(cross(UpVector, N));
    vec3 TangentY = cross(N, TangentX);
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

float3 PrefilterEnvMap(float Roughness, float3 R)
{
    float3 N = normalize(R);
    float3 V = normalize(R);
    float3 PrefilteredColor = 0;
    float TotalWeight = 0.0;
    const uint NumSamples = 1024;
    for (uint i = 0; i < NumSamples; i++)
    {
        float2 Xi = hammersley(i, NumSamples);
        float3 H = importanceSampleGGX(Xi, Roughness, N);
        float3 L = 2 * dot(V, H) * H - V;
        float NoL = saturate(dot(N, L));
        if (NoL > 0)
        {
            PrefilteredColor += gEnvMap.SampleLevel(gsamLinear, normalize(L), 0.0).rgb * NoL;
            TotalWeight += NoL;
        }
    }
    return PrefilteredColor / TotalWeight;
}

// Karis 2014
vec2 integrateBRDF(float roughness, float NoV)
{
    vec3 V;
    V.x = sqrt(1.0 - NoV * NoV); // sin
    V.y = 0.0;
    V.z = NoV; // cos

    // N points straight upwards for this integration
    const vec3 N = vec3(0.0, 0.0, 1.0);

    float A = 0.0;
    float B = 0.0;
    const uint numSamples = 1024u;

    for (uint i = 0u; i < numSamples; i++) {
        vec2 Xi = hammersley(i, numSamples);
        // Sample microfacet direction
        vec3 H = importanceSampleGGX(Xi, roughness, N);

        // Get the light direction
        vec3 L = 2.0 * dot(V, H) * H - V;

        float NoL = saturate(dot(N, L));
        float NoH = saturate(dot(N, H));
        float VoH = saturate(dot(V, H));

        if (NoL > 0.0) {
            float V_pdf = V_SmithGGXCorrelated(NoV, NoL, roughness) * VoH * NoL / NoH;
            float Fc = pow(1.0 - VoH, 5.0);
            A += (1.0 - Fc) * V_pdf;
            B += Fc * V_pdf;
        }
    }

    return 4.0 * vec2(A, B) / float(numSamples);
}

float3 ApproximateSpecularIBL(float3 N, float3 V, float3 baseColor, float roughness, float metallic)
{
    roughness = 0.2;
    // return specularIBL(N, V, baseColor, metallic, roughness, 1024);
    float NoV = saturate(dot(N, V));
    float3 R = 2 * dot(V, N) * N - V; // ??? what is this
    float2 brdf = integrateBRDF(roughness, NoV); // gBrdfMap.Sample(gsamLinear, float2(roughness, NoV)).rg;
    // float mipLevels = 4;
    // float3 color = gEnvMap.SampleLevel(gsamLinear, normalize(N), 1.0).rgb;
    float3 radiance = PrefilterEnvMap(roughness, normalize(R));
    float3 f0 = lerp(float3(0.04, 0.04, 0.04), baseColor, metallic);
    return radiance * (f0 * brdf.x + brdf.y);
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
    // return ApproximateDiffuseIBL(N, (1.0-metallic)*baseColor) + ApproximateSpecularIBL(N, V, baseColor, roughness, metallic);
    return ApproximateSpecularIBL(N, V, baseColor, roughness, metallic);
}

