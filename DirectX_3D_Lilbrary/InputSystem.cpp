#include "pch.h"
#include "InputSystem.h"

constexpr float ROTATION_GAIN = 0.004F;
constexpr float MOVEMENT_GAIN = 0.07f;

InputSystem::InputSystem() : m_MouseState(), m_KeyboardState()
{
    assert(instance == nullptr);
    instance = this;
}

InputSystem* InputSystem::instance = nullptr;

void InputSystem::Update(float DeltaTime)
{
    m_MouseState = m_Mouse->GetState();
    m_MouseButtonStateTracker.Update(m_MouseState);

    m_KeyboardState = m_Keyboard->GetState();
    m_KeyboardStateTracker.Update(m_KeyboardState);

    if (nullptr != m_pInputProcessor)
    {
        m_pInputProcessor->OnInputProcess(m_KeyboardState, m_KeyboardStateTracker, m_MouseState, m_MouseButtonStateTracker);
    }
}

bool InputSystem::Initialize(HWND hWnd, InputProcessor* processor)
{
    m_Keyboard = std::make_unique<DirectX::Keyboard>();
    m_Mouse = std::make_unique<DirectX::Mouse>();
    m_Mouse->SetWindow(hWnd);
    m_pInputProcessor = processor;
    return true;
}
