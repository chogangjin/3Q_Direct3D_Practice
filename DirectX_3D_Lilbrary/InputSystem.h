#pragma once

#include <directxtk/Mouse.h>
#include <directxtk/Keyboard.h>

class InputProcessor
{
public:
	virtual void OnInputProcess(const DirectX::Keyboard::State& keyboard, const DirectX::Keyboard::KeyboardStateTracker& keyboardTracker,
		const DirectX::Mouse::State& mousestate, const DirectX::Mouse::ButtonStateTracker& mousebuttonstatetracker) = 0;
};

class InputSystem
{
public:
	InputSystem();
	~InputSystem() {}

	static InputSystem* instance;

	InputProcessor* m_pInputProcessor = nullptr;
	
	std::unique_ptr<DirectX::Keyboard> m_Keyboard;
	std::unique_ptr<DirectX::Mouse> m_Mouse;

	DirectX::Keyboard::KeyboardStateTracker m_KeyboardStateTracker;
	DirectX::Mouse::ButtonStateTracker m_MouseButtonStateTracker;
	DirectX::Mouse::State m_MouseState;
	DirectX::Keyboard::State m_KeyboardState;

	void Update(float DeltaTime);
	bool Initialize(HWND hWnd, InputProcessor* processor);
};

