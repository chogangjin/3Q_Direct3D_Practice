#include "Shared.fxh"

PS_INPUT main(VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    float4 pos = input.pos;
    float4x4 mworld = mul(BoneOffset[RefBoneIndex], World);
    if (IsRigid == false)
    {
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
        
        mworld = mul(WeightOffsetPose, World);
    }
    pos = mul(pos, mworld);
    pos = mul(pos, ShadowView);
    pos = mul(pos, ShadowProjection);
    output.pos = pos;

    return output;
}