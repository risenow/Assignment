#include "basicvsconstants.h"
#include "BasicPSConsts.h"

SamplerState Sampler;
Texture2D diffuseMap: register(t0);

struct PS_OUTPUT
{
    float4 Color: SV_Target0;
#ifdef GBUFFER_PASS
    float4 Normal: SV_Target1;
#endif
};

PS_OUTPUT PSEntry(
    in float4 pos : SV_POSITION,
    in float4 vpos : POSITION1,
    in float4 normal : NORMAL1,
    in float2 tc : TEXCOORD1) : SV_TARGET
{
    PS_OUTPUT output;
    output.Color = diffuseMap.Sample(Sampler, float2(tc.x, 1.0f - tc.y));
#ifdef GBUFFER_PASS
    float4 n = (float4(mul((float3x3)view, normalize(float3(normal.x, normal.y, normal.z))), 1.0f) + 1.0f) * 0.5f;
    n.w = shin/128.0f;

    output.Normal = n;
#endif
    return output;
}