#include "Shared.fxh"

float3 LineartoSRGB(float3 linearColor)
{
    return pow(linearColor, 1.0f / 2.2f);
}

float4 main(PS_QUAD_INPUT input) : SV_TARGET
{
    float3 C_linear709 = txHDRSceneTexture.Sample(samLinear, input.UV).rgb;
    
    float exposureFactor = pow(2.0f, Exposure);
    C_linear709 *= exposureFactor;
    
    float3 C_tonemapped;
    C_tonemapped = ACESFilm(C_linear709);
    
    float3 C_final;
    C_final = LineartoSRGB(C_tonemapped);
    
    return float4(C_final, 1.0);
}