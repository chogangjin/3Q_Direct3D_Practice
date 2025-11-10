#pragma once
#include "framework.h"
//#include <directxtk/SimpleMath.h>
using namespace DirectX::SimpleMath;

struct KeyFrame // 프레임 하나하나
{
	float Time;
	Vector3 Position;
	Quaternion  Rotation;
	Vector3 Scale;
};

struct BoneAnimation // 프레임들을 모아둔 본 한개짜리의 애니메이션 
{
	std::vector<KeyFrame> AnimationKeys;
	std::string m_Name;
	//보간을 해서 프레임 사이의 변환값 만들기
	void Evaluate(float time, Vector3& position, Quaternion& rotation, Vector3& scale);
};

struct Animation // 모든 본의 애니메이션 정보를 모아둔 애니메이션 그 자체
{
	std::string m_Name;
	double m_Duration;
	std::vector<BoneAnimation> m_BoneAnimations; // 각 부위별 애니메이션
};