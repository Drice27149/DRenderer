#define vec3 float3
#define vec2 float2
#define PI 3.1415926

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
    float CosTheta = sqrt((1.0 - Xi.y) / (1.0 + (/*a * a*/ a - 1.0) * Xi.y));
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
    const uint NumSamples = 4096;
    for (uint i = 0; i < NumSamples; i++)
    {
        float2 Xi = hammersley(i, NumSamples);
        float3 H = importanceSampleGGX(Xi, Roughness, N);
        float3 L = 2 * dot(V, H) * H - V;
        float NoL = saturate(dot(N, L));
        if (NoL > 0)
        {
            PrefilteredColor += gCubeMap.SampleLevel(gsamLinear, normalize(L), 0.0).rgb * NoL;
            TotalWeight += NoL;
        }
    }
    return PrefilteredColor / TotalWeight;
}
