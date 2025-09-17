#pragma once
#include "framework.h"
//#include "Resource.h"
#include "TimeSystem.h"
#include "InputSystem.h"
#include "Camera.h"

class GameApp
{
	HINSTANCE hInst;

	std::wstring m_Title = L"KangJin_D3D";
	std::wstring m_WindowClass = L"KangJinWindow";
	std::string m_ModulePath;
	std::string m_WorkingPath;
	bool m_bQuit = false;

public:
	GameApp();
	virtual ~GameApp();
	
	int m_Width = 1600;
	int m_Height = 1200;
	
	static HWND m_handleWindow;
	static GameApp* m_pInstance;
	
	virtual bool Initialize();
	virtual void LateInitialize();
	virtual void Shutdown();
	virtual void MessageProc(HWND, UINT, WPARAM, LPARAM);
	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParameter, LPARAM lParameter);

	TimeSystem m_pTime;
	InputSystem m_Input;
	Camera m_Camera;
	void Run();
	
	ATOM MyRegisterClass(HINSTANCE hInstance);
	BOOL InitInstance(HINSTANCE, int);
	static HWND& GetHandleWindow() { return m_handleWindow; }
	int GetWidth() { return m_Width; }
	int GetHeight() { return m_Height; }

	void QuitGame() { m_bQuit = true; }
	virtual void Render() = 0;
protected :
	virtual void OnUpdate() = 0;
	void FixedUpdate();
private:
	void Update();
};

