#include "Shared.fxh"

PS_SKYBOX_INPUT main(VS_SKYBOX_INPUT pos)
{
    PS_SKYBOX_INPUT output;
    
    output.pos = mul(pos.pos, World);
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    output.pos = output.pos.xyww;
    
    output.Tex = (float3) pos.pos;
    
	return output;
}