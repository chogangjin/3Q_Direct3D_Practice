#include "Shared.fxh"

PS_SKYBOX_INPUT main(VS_SKYBOX_INPUT pos)
{
    PS_SKYBOX_INPUT output;
        
    matrix ViewMatrix = View; // 카메라 행렬 제거하기 위한 행렬
    ViewMatrix._41 = 0;
    ViewMatrix._42 = 0;
    ViewMatrix._43 = 0;
    
    output.pos = mul(pos.pos, World);
    output.pos = mul(output.pos, ViewMatrix);
    output.pos = mul(output.pos, Projection);
    
    output.Tex = pos.pos.xyz;
    
    
    
	return output;
}