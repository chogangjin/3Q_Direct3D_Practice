#include <Shared.fxh>
float4 main(PS_INPUT input) : SV_TARGET
{
    float4 phong = 0;
    for (int i = 0; i < 2; i++)
    {
        phong += saturate(dot((float3) vDirectionalLight[0], input.norm) * vDirectionalColor[0]);
    }
    phong.a = 1;
    return phong * txDiffuse.Sample(samLinear, input.Tex);
}