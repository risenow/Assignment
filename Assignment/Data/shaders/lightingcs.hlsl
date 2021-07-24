#include "DeferredLightingConsts.h"

SamplerComparisonState ShadowSampler : register(s0);

Texture2D<float> Depth : register(t0);
Texture2D<float4> Normals : register(t1);
Texture2D<float4> Albedo : register(t2);
Texture2D<float4> ShadowMap : register(t3);

RWTexture2D<float4> OutColor : register(u0);

float2 ndcToFlippedUV(float2 ndc)
{
    float2 uv = float2(ndc.x * 0.5f + 0.5f, ndc.y * (-0.5f) + 0.5f);

    return float2(uv.x, uv.y);
}

float2 TCtoNDC(float2 uv)
{
    float2 ndc = uv * 2.0f - float2(1.0f, 1.0f);

    return float2(ndc.x, ndc.y * (-1.0f));
}


float4 CorrectGamma(float4 color)
{
#ifdef GAMMA_CORRECTION
    return pow(color, 1.0 / 2.2);
#else
    return color;
#endif
}

float4 UnCorrectGamma(float4 color)
{
#ifdef GAMMA_CORRECTION
    return pow(color, 2.2);
#else
    return color;
#endif
}

[numthreads(8, 8, 1)]
void CSEntry(uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID)
{
    uint width;
    uint height;

    Albedo.GetDimensions(width, height);

    float2 uv = float2((float)DTid.x / width, (float)DTid.y / height);
    float2 dNDC = TCtoNDC(uv);

    uint3 uTC = uint3(DTid.x, DTid.y, 0);

    float depth = Depth.Load(uTC);

    float4 ndc = float4(dNDC.x, dNDC.y, depth, 1.0f);
    float4 vPos = mul(unproj, ndc);
    vPos = vPos/ vPos.w;

    float3 albedo = UnCorrectGamma(Albedo.Load(uTC)).rgb;

    float4 wPos = mul(unview, vPos);

    float4 shadow_vPos = mul(shadowview, wPos);
    float4 shadow_pPos = mul(shadowproj, shadow_vPos);
    shadow_pPos /= shadow_pPos.w;

    float shadow_Z = shadow_pPos.z;

    float2 shadow_NDC = shadow_pPos.xy;
    float2 shadow_UV = ndcToFlippedUV(shadow_NDC);

    float shadowFactor = min((ShadowMap.SampleCmpLevelZero(ShadowSampler, shadow_UV, shadow_Z - 0.01).r + 0.35), 1.0);

    float4 normalShin = Normals.Load(uTC);//(Normals.Load(uTC))*2.0f - 1.0f;
    float shin = normalShin.w * 128.0f;
    shin = shin == 0.0f ? 70.0f : shin;

    float3 normal = normalize(normalShin.xyz * 2.0f - 1.0f);

    float3 lightVec = normalize(lightDir.xyz);

    float3 r = normalize(reflect(lightVec, normal));

    const float3 spec = float3(1.0, 1.0f, 1.0f);
    const float ambCoef = 0.35f;

    float3 v = normalize(-vPos.xyz);
    float specFactor = max(dot(r, v), 0.0f);
    float specFactorPow = pow(specFactor, shin);

    float3 amb = ambCoef * albedo;
    float3 lit = specFactorPow * spec + albedo * max(dot(normal, -lightVec), 0.0f);

    OutColor[uTC.xy] = CorrectGamma(float4(shadowFactor * lit + amb, 1.0f));
}