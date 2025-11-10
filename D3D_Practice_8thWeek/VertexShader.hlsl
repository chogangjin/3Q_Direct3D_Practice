#include <Shared.fxh>
PS_INPUT main( VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    //matrix mworld = mul(BoneOffset[RefBoneIndex], World);
    float4 pos = input.pos;
    
    // v * offset * pose 중에서 offset과 pose를 먼저 계산
    float4x4 OffsetPose[4];
    OffsetPose[0] = mul(BoneOffset[input.BlendIndices.x], BonePose[input.BlendIndices.x]);
    OffsetPose[1] = mul(BoneOffset[input.BlendIndices.y], BonePose[input.BlendIndices.y]);
    OffsetPose[2] = mul(BoneOffset[input.BlendIndices.z], BonePose[input.BlendIndices.z]);
    OffsetPose[3] = mul(BoneOffset[input.BlendIndices.w], BonePose[input.BlendIndices.w]);
    
    float4x4 WeightOffsetPose;
    WeightOffsetPose = mul(input.BlendWeights.x, OffsetPose[0]);
    WeightOffsetPose += mul(input.BlendWeights.y, OffsetPose[1]);
    WeightOffsetPose += mul(input.BlendWeights.z, OffsetPose[2]);
    WeightOffsetPose += mul(input.BlendWeights.w, OffsetPose[3]);
    
    Matrix ModelToWorld = mul(WeightOffsetPose, World);
    
    output.pos = mul(pos, ModelToWorld);
    output.worldpos = output.pos.xyz;
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    output.Tex = input.Tex;
    output.norm = normalize(mul(input.normal, (float3x3) ModelToWorld));
    output.tangent = normalize(mul(input.tangent, (float3x3) ModelToWorld));
    output.binormal = normalize(mul(input.binormal, (float3x3) ModelToWorld));
	return output;
}