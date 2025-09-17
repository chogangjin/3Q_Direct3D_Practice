#include "framework.h"
#include "GameApp.h"

GameApp* GameApp::m_pInstance = nullptr;
HWND GameApp::m_handleWindow = nullptr;
LRESULT CALLBACK DefaultWndProc(HWND hWnd, UINT message, WPARAM wParameter, LPARAM lParameter);
GameApp::GameApp() : hInst(nullptr)
{
	m_pInstance = this;
}

GameApp::~GameApp()
{
}

bool GameApp::Initialize()
{
	AllocConsole();
	FILE* pFile;
	freopen_s(&pFile, "CONOUT$", "w", stdout);

	hInst = GetModuleHandle(NULL); // NULL이 들어가는 이유는, 현재 작동중인 프로그램의 핸들을 얻기 위해서

	WNDCLASSEXW wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = DefaultWndProc;
	wcex.hInstance = hInst;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = m_WindowClass.c_str(); //LPCSSTR은 wchar_t로 받을 수 있으니, wstring을 wchar_t로 바꿔주는 c_str()사용해야 함

	//RegisterClassExA(ANSI)또는 RegisterClassEXW(UNICODE) 둘중 하나를 선택해서 호출하는 매크로
	RegisterClassEx(&wcex); // 기존에는 이 함수로 리턴시켰는데, 그냥 이 함수 호출 시키면 될듯?

	//원하는 크기로 사이즈 조정
	SIZE windowSize = { m_Width,m_Height };
	RECT windowRect = { 0,0,windowSize.cx, windowSize.cy };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	m_handleWindow = CreateWindowEx(
		0,
		m_WindowClass.c_str(), // 윈도우 창 이름
		m_Title.c_str(), // 창 제목 설정
		WS_OVERLAPPEDWINDOW, // 설정 값(overlapped? 아니면 조그마한 창? 등등 여러가지 설정 정할 수 있음)
		CW_USEDEFAULT, CW_USEDEFAULT, // x값, y 값
		windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, // 너비와 높이 설정
		NULL,		//부모클래스 설정
		NULL,		//메뉴 설정
		hInst,// 창이 받을  instance 설정
		this		// voidPointer이기 때문에 어떤 클래스나 포인터로 받을 수 있음
	);
	//SetConsoleActiveScreenBuffer(m_handleWindow);

	ShowWindow(m_handleWindow, SW_SHOW);
	UpdateWindow(m_handleWindow);
	CoInitialize(nullptr);

	TimeSystem::GetInstance()->Initialize();
	m_Input.Initialize(m_handleWindow, &m_Camera);

	return true;
}

void GameApp::LateInitialize()
{
}

void GameApp::Shutdown()
{
	CoUninitialize();
}

void GameApp::MessageProc(HWND hWnd, UINT message, WPARAM wParameter, LPARAM lParameter)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
}
LRESULT CALLBACK GameApp::WndProc(HWND hWnd, UINT message, WPARAM wParameter, LPARAM lParameter)
{

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_ACTIVATEAPP:
		DirectX::Keyboard::ProcessMessage(message, wParameter, lParameter);
		DirectX::Mouse::ProcessMessage(message, wParameter, lParameter);
		break;
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		DirectX::Mouse::ProcessMessage(message, wParameter, lParameter);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		DirectX::Keyboard::ProcessMessage(message, wParameter, lParameter);
		break;
	default:
		return DefWindowProc(hWnd, message, wParameter, lParameter);
	}
	return 0;
}

void GameApp::Run()
{
	MSG message;
	while (!m_bQuit)
	{
		if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT) { break; }

			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		else
		{
			Update();
			Render();
		}
	}
}

ATOM GameApp::MyRegisterClass(HINSTANCE hInstance)
{
	return ATOM();
}

BOOL GameApp::InitInstance(HINSTANCE, int)
{
	return 0;
}

void GameApp::Update()
{
	TimeSystem::GetInstance()->Update();
	m_Input.Update(m_pTime.deltaTime);
	m_Camera.Update(m_pTime.deltaTime);

	OnUpdate();
}

void GameApp::FixedUpdate()
{
}

void GameApp::Render()
{

}

LRESULT CALLBACK DefaultWndProc(HWND hWnd, UINT message, WPARAM wParameter, LPARAM lParameter)
{
	return GameApp::m_pInstance->WndProc(hWnd,message, wParameter, lParameter);
}
