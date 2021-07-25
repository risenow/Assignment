#include "basicvsconstants.h"
#include "SkyBoxConsts.h"


struct PS_OUTPUT
{
    float4 Color: SV_Target0;
};

float3 GetSpherePos(float3 boxPos, float4 skySphere) //invariant: sphere center is center of the box
{
    float3 toSphere = normalize(boxPos - skySphere.xyz) * skySphere.w;

    return skySphere.xyz + toSphere;
}

float SphericalDistance(float3 p1, float3 p2, float4 skySphere)
{
    const float pi = 3.14;

    float3 v1 = normalize(p1 - skySphere.xyz);
    float3 v2 = normalize(p2 - skySphere.xyz);
    float alpha = acos(dot(v1, v2));

    return skySphere.w * alpha;
}

float SunIntensity(float4 sunPos, float3 boxPos, float4 skySphere) //invariant: sunPos is on the skySphere
{
    float3 skyPos = GetSpherePos(boxPos, skySphere);
    float dist = SphericalDistance(skyPos, sunPos.xyz, skySphere);

    return sunPos.w / dist;
}

PS_OUTPUT PSEntry(
    in float4 pos : SV_POSITION,
    in float4 vpos : POSITION1,
    in float4 normal : NORMAL1,
    in float2 tc : TEXCOORD1) : SV_TARGET
{
    PS_OUTPUT output;

    float3 sunColor = float3(1.0f, 1.0f, 0.0f);
    float3 skyColor = float3(0.2f, 0.8f, 1.0f); 

    output.Color = float4(SunIntensity(SunPos, vpos, SkySphere) * sunColor + skyColor, 1.0f);
    return output;
}