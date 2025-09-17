#include "pch.h"
#include "Camera.h"

Camera::Camera()
{
    Reset();
}

Vector3 Camera::GetForward()
{
    return -m_WorldMatrix.Forward();
}

Vector3 Camera::GetRight()
{
    return m_WorldMatrix.Right();
}

void Camera::Reset()
{
    m_WorldMatrix = Matrix::Identity;
    m_Rotation = Vector3(0, 0, 0);
    m_Position = m_InitialPosition;
}

void Camera::Update(float elapsedTime)
{
    if (m_InputVector.Length() > 0.0f)
    {
        m_Position += m_InputVector * m_MoveSpeed * elapsedTime;
        m_InputVector = Vector3{ 0,0,0 };
    }
    
    m_WorldMatrix = Matrix::CreateFromYawPitchRoll(m_Rotation) *
        Matrix::CreateTranslation(m_Position);
}

void Camera::GetViewMatrix(Matrix& out)
{
    Vector3 eye = m_WorldMatrix.Translation();
    Vector3 target = m_WorldMatrix.Translation() + GetForward();
    Vector3 up = m_WorldMatrix.Up();
    out = XMMatrixLookAtLH(eye, target, up);
}

void Camera::AddInputVector(const Vector3& input)
{
    m_InputVector += input;
    m_InputVector.Normalize();
}

void Camera::AddPitch(float value)
{
    m_Rotation.x += value;
    if (m_Rotation.x > XM_PI)
    {
        m_Rotation.x -= XM_2PI;
    }
    else if (m_Rotation.x < -XM_PI)
    {
        m_Rotation.x += XM_2PI;
    }
}

void Camera::AddYaw(float value)
{
    m_Rotation.y += value;
	if (m_Rotation.y > XM_PI)
	{
		m_Rotation.y -= XM_2PI;
	}
	else if (m_Rotation.y < -XM_PI)
	{
		m_Rotation.y += XM_2PI;
	}
}

void Camera::OnInputProcess(const DirectX::Keyboard::State& keyboard, const DirectX::Keyboard::KeyboardStateTracker& keyboardTracker, const DirectX::Mouse::State& mousestate, const DirectX::Mouse::ButtonStateTracker& mousebuttonstatetracker)
{
    Vector3 forward = GetForward();
    Vector3 right = GetRight();

    if (keyboardTracker.IsKeyPressed(DirectX::Keyboard::Keys::R))
    {
        Reset();
    }

    if (keyboard.IsKeyDown(DirectX::Keyboard::Keys::W))
    {
        AddInputVector(forward);
    }
    else if (keyboard.IsKeyDown(DirectX::Keyboard::Keys::S))
    {
        AddInputVector(-forward);
    }

    if (keyboard.IsKeyDown(DirectX::Keyboard::Keys::A))
    {
        AddInputVector(-right);
    }
    else if (keyboard.IsKeyDown(DirectX::Keyboard::Keys::D))
    {
        AddInputVector(right);
    }

    if (keyboard.IsKeyDown(DirectX::Keyboard::Keys::E))
    {
        AddInputVector(-m_WorldMatrix.Up());
    }
    else if (keyboard.IsKeyDown(DirectX::Keyboard::Keys::Q))
    {
		AddInputVector(m_WorldMatrix.Up());
    }

    if (keyboard.IsKeyDown(DirectX::Keyboard::Keys::F1))
    {
        SetSpeed(250);
    }
    else if (keyboard.IsKeyDown(DirectX::Keyboard::Keys::F2))
    {
		SetSpeed(500);
    }
    else if (keyboard.IsKeyDown(DirectX::Keyboard::Keys::F3))
    {
		SetSpeed(1000);
    }

    InputSystem::instance->m_Mouse->SetMode(mousestate.rightButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);
    if (mousestate.positionMode == Mouse::MODE_RELATIVE)
    {
        Vector3 delta = Vector3(float(mousestate.x), float(mousestate.y), 0.0f) * m_RotationSpeed;
        AddPitch(delta.y);
        AddYaw(delta.x);
    }
}
