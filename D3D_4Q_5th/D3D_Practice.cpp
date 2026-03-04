// D3D_Practice.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "D3D_Practice.h"
#include "DemoGameApp.h"
// 전역 변수:

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    DemoGameApp demogameapp;
    demogameapp.Initialize();
    demogameapp.LateInitialize();
    demogameapp.Run();
    demogameapp.Shutdown();
}

