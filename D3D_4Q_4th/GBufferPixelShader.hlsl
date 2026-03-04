#include "Shared.fxh"

struct GBufferOut
{
    float4 BaseColor : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 Position : SV_TARGET2;
};


GBufferOut main(VS_GBUFFER_OUTPUT input)
{
    GBufferOut output;

    float3 albedo = txDiffuse.Sample(samLinear, input.UV).rgb;

    float3 N = normalize(input.norm);

    //if (HasNormalMap != 0)
    if (HasNormalMap != 0)
    {
        float3 nTS = txNormal.Sample(samLinear, input.UV).rgb;
        nTS = normalize(nTS * 2.0f - 1.0f);

        float3 T = normalize(input.tangent);
        float3 B = normalize(input.binormal);

        float3x3 TBN = float3x3(T, B, N); // 컬럼 기반
        N = normalize(mul(TBN, nTS)); // TS -> WS
    }

    float roughness = txRoughness.Sample(samLinear, input.UV).r;
    float metalic = txMetalic.Sample(samLinear, input.UV).r;
    float Opacity = txOpacity.Sample(samLinear, input.UV).r;
    if (OverrideMaterial != 0)
    {
        roughness = Roughness;
        metalic = Metalness;
    }

    output.BaseColor = float4(albedo, 1.0f);
    output.Normal = float4(N * 0.5f + 0.5f, roughness);
    //output.Normal = float4(N * 0.5f + 0.5f, 1.0f);
    output.Position = float4(input.worldpos, metalic);
    //output.Position = float4(input.worldpos, 1.0f);

    return output;
}