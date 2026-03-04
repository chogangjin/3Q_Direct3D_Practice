#include "Shared.fxh"
		

PS_BILLBOARDINPUT main(VS_BILLBOARDINPUT input)
{
    PS_BILLBOARDINPUT output;
    
    output.Position = mul(float4(input.Position, 1), BillBoardWorld);
    output.Position = mul(output.Position, BillBoardView);
    output.Position = mul(output.Position, BillBoardProjection);
    output.Color = input.Color;
    output.UV = input.UV * UVScale + UVOffset;
    
    return output;
}