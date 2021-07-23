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
    in float4 normal : NORMAL1,
    in float2 tc : TEXCOORD1) : SV_TARGET
{
    PS_OUTPUT output;
    output.Color = diffuseMap.Sample(Sampler, tc);
#ifdef GBUFFER_PASS
    output.Normal = float4(normal.x, normal.y, normal.z, 1.0f);
#endif
    return output;
}