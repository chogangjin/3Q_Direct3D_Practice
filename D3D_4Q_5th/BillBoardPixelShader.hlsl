#include "Shared.fxh"

float4 main(PS_BILLBOARDINPUT input) : SV_TARGET
{
    float4 pos = input.Position;
    float4 billboardTexture = txBillBoard.Sample(samLinear, input.UV);
    
    // use Clip
    //clip(billboardTexture.r + billboardTexture.g + billboardTexture.b - 0.1f);
    
    float birghtness = max(billboardTexture.r, max(billboardTexture.g, billboardTexture.b));
    billboardTexture.a = birghtness;
    return billboardTexture;
}