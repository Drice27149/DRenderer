#define PI 3.1415926

// NoH: n dot h, a: alpha (roughness?)
float D_GGX(float NoH, float roughness) {
    float oneMinusNoHSquared = 1.0 - NoH * NoH;
    float a = NoH * roughness;
    float k = roughness / (oneMinusNoHSquared + a * a);
    float d = k * k * (1.0 / PI);
    return d;
}

float3 F_Schlick(float u, float3 f0) {
    return f0 + (float3(1.0, 1.0, 1.0) - f0) * pow(1.0 - u, 5.0);
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
    float a2 = a * a;
    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (GGXV + GGXL);
}

float Fd_Lambert() {
    return 1.0 / PI;
}

float3 BRDF_Faliment(float3 N, float3 V, float3 L, float3 baseColor, float metallic, float roughness)
{
    float3 H = normalize(V + L);
    float NoV = abs(dot(N, V)) + 0.00001;
    float NoL = clamp(dot(N, L), 0.0, 1.0);
    float NoH = clamp(dot(N, H), 0.0, 1.0);
    float LoH = clamp(dot(L, H), 0.0, 1.0);
    // UE4: roughness = pow(roughness, 4)
    roughness = roughness * roughness;
    // UE4: 0.04 -> 0.08
    float3 f0 = lerp(float3(0.04, 0.04, 0.04), baseColor, metallic);
    float3 diffuseColor = (1.0 - metallic) * baseColor;

    float D = D_GGX(NoH, roughness);
    float3 F = F_Schlick(LoH, f0);
    float Vis = V_SmithGGXCorrelated(NoV, NoL, roughness);

    // specular BRDF, 还有一种表达方式是 D*F*G/4(~), 这里 V = G/4(~)
    float3 Fr = (D * Vis) * F;
    // diffuse BRDF
    float3 Fd = diffuseColor * Fd_Lambert();
    // apply lighting...
    return Fd + Fr;
}