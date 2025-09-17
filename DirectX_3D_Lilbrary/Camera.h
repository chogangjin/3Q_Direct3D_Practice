#pragma once

#include <directxtk/SimpleMath.h>
#include "InputSystem.h"

using namespace DirectX::SimpleMath;

class Camera : public InputProcessor
{
public:
	Camera();
	Vector3 m_Rotation;
	Vector3 m_InitialPosition = { 0,0,-30 };
	Vector3 m_Position;
	Matrix m_WorldMatrix;
	Vector3 m_InputVector;
	
	float m_MoveSpeed = 20.0f;
	float m_RotationSpeed = 0.004f;

	Vector3 GetForward();
	Vector3 GetRight();

	void Reset();
	void Update(float elapsedTime);
	void GetViewMatrix(Matrix& out);
	void AddInputVector(const Vector3& input);
	void SetSpeed(float speed) { m_MoveSpeed = speed; }
	void AddPitch(float value);
	void AddYaw(float value);

	virtual void OnInputProcess(const DirectX::Keyboard::State& keyboard, const DirectX::Keyboard::KeyboardStateTracker& keyboardTracker,
		const DirectX::Mouse::State& mousestate, const DirectX::Mouse::ButtonStateTracker& mousebuttonstatetracker);
};

