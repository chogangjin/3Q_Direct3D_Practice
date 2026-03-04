#include "Shared.fxh"

float3 Rec7091ToRec2020(float3 color)
{
    static const float3x3 conversion =
    {
        0.627402, 0.329292, 0.043306,
        0.069095, 0.919544, 0.011360,
        0.016394, 0.088028, 0.895578
    };

    return mul(conversion, color);
}

float3 LinearToST2004(float3 color)
{
    float m1 = 2610.0 / 4096.0 / 4;
    float m2 = 2523.0 / 4096.0 * 128;
    float c1 = 3424.0 / 4096.0;
    float c2 = 2413.0 / 4096.0 * 32;
    float c3 = 2392.0 / 4096.0 * 32;
    float3 cp = pow(abs(color), m1);
    
    return pow((c1 + c2 * cp) / (1 + c3 * cp), m2);
}
float3 LinearToST2084(float3 color)
{
    float m1 = 2610.0 / 4096.0 / 4;
    float m2 = 2523.0 / 4096.0 * 128;
    float c1 = 3424.0 / 4096.0;
    float c2 = 2413.0 / 4096.0 * 32;
    float c3 = 2392.0 / 4096.0 * 32;
    float3 cp = pow(abs(color), m1);
    return pow((c1 + c2 * cp) / (1 + c3 * cp), m2);
}

float4 main(PS_QUAD_INPUT input) : SV_TARGET
{
    float3 C_linear709 = txHDRSceneTexture.Sample(samLinear, input.UV).rgb;
    float3 C_exposure = C_linear709 * pow(2.0f, Exposure);
    float3 C_tonemapped = ACESFilm(C_exposure);
    
    const float st2084max = 10000.0;
    const float hdrScalar = MaxHDRNits / st2084max;
    float3 C_rec2020 = Rec7091ToRec2020(C_tonemapped);
    float3 C_ST2084 = LinearToST2004(C_rec2020 * hdrScalar);
    float4 finalcolor = float4(C_ST2084, 1.0);
    return finalcolor;
}
