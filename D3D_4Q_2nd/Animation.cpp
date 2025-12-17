#include "Animation.h"

void BoneAnimation::Evaluate(double time, Vector3& position, Quaternion& rotation, Vector3& scale)
{
	// 키프레임 시간이 아니라, 델타타임 기준으로 트랜스폼 값 보간
	double currentTime = fmod(time, AnimationKeys.back().Time);
	int frameIndex = 0;
	for (int i = 0; i < AnimationKeys.size()-1; i++)
	{
		if (currentTime < AnimationKeys[i + 1].Time)
		{
			frameIndex = i;
			break;
		}
	}
	int nextIndex = frameIndex + 1;

	double deltaFrame = AnimationKeys[nextIndex].Time - AnimationKeys[frameIndex].Time;
	double rate = (currentTime- AnimationKeys[frameIndex].Time) / deltaFrame;
	
	position = Vector3::Lerp(AnimationKeys[frameIndex].Position, AnimationKeys[nextIndex].Position, (float)rate);
	rotation = Quaternion::Slerp(AnimationKeys[frameIndex].Rotation, AnimationKeys[nextIndex].Rotation, (float)rate);
	scale = Vector3::Lerp(AnimationKeys[frameIndex].Scale, AnimationKeys[nextIndex].Scale, (float)rate);
}
