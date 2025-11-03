#include <Shared.fxh>
PS_INPUT main( VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    matrix mworld = mul(ModelMatricies[RefBoneIndex], World);
    float4 pos = input.pos;
    output.pos = mul(pos, mworld);
    output.worldpos = output.pos.xyz;
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    output.Tex = input.Tex;
    output.norm = normalize(mul(input.normal, (float3x3) mworld));
    output.tangent = normalize(mul(input.tangent, (float3x3) mworld));
    output.binormal = normalize(mul(input.binormal, (float3x3) mworld));
	return output;
}