#include "Shared.fxh"

float4 main(PS_SKYBOX_INPUT input) : SV_TARGET
{
    return SkyBox.Sample(samLinear, normalize(input.Tex));
}