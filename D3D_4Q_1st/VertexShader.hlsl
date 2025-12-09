#include <Shared.fxh>
PS_INPUT main( VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    output.pos = mul(input.pos, World);
    output.worldpos = output.pos;
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    output.Tex = input.Tex;
    output.norm = normalize(mul(input.normal, (float3x3) World));
    output.tangent = normalize(mul(input.tangent, (float3x3) World));
    output.binormal = normalize(mul(input.binormal, (float3x3) World));
	return output;
}