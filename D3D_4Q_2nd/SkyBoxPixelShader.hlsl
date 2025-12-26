#include "Shared.fxh"

float4 main(PS_SKYBOX_INPUT input) : SV_TARGET
{
    return txEnvironmentMap.Sample(samLinear, normalize(input.Tex));
}