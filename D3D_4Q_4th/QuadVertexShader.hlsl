#include "Shared.fxh"

PS_QUAD_INPUT main(VS_QUAD_INPUT input)
{
    PS_QUAD_INPUT output;
    output.pos = float4(input.pos.xy, 0, 1);
    output.UV= input.Tex;
	return output;
}