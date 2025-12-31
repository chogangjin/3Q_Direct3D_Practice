#pragma once
#include "framework.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;

struct SkyBoxVertex
{
	Vector3 position;
	Vector2	texture;
	Vector3 Normal;
	Vector3 Tangent;
	Vector3 BiNormal;

	SkyBoxVertex(Vector3 position, Vector2 texture = {}, Vector3 Normal = {}, Vector3 tangent = {}, Vector3 binormal = {}) : position(position), texture(texture), Normal(Normal), Tangent(tangent), BiNormal(binormal) {}
};

