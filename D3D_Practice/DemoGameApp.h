#pragma once
#include "../DirectX_3D_Lilbrary/GameApp.h"
#include <d3d11.h>
#include <wrl.h>
#include <directxtk/Simplemath.h>

#include <imgui.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;

class DemoGameApp : public GameApp
{
public :
	DemoGameApp();
	~DemoGameApp();

	// 렌더링 파이프라인을 구성하는 필수 객체의 인터페이스
	ComPtr<ID3D11Device> m_pDevice = nullptr;   // d3d device
	ComPtr<ID3D11DeviceContext> m_pDeviceContext = nullptr; // d3d deviceContext
	ComPtr<IDXGISwapChain1> m_pSwapChain = nullptr; // SwapChain
	ComPtr<IDXGIDevice3> m_dxgiDevice = nullptr;
	ComPtr<IDXGIAdapter3> m_pDxgiAdapter = nullptr;
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView = nullptr; // RenderTarget view
	ComPtr<ID3D11DepthStencilView> m_pDepthStencilView = nullptr; // Depth/Stencil view
	ComPtr<ID3D11DepthStencilState> m_pDepthStencilState = nullptr;
	ComPtr<ID3D11DepthStencilState> m_pSkyBoxDepthStencilState = nullptr; // Depth/Stencil state
	ComPtr<ID3D11RasterizerState> m_pRasterizerState = nullptr;
	ComPtr<ID3D11BlendState> m_pAlphaBlendState = nullptr;

	//렌더링 파이프라인에 사용되는 정보들 
	ComPtr<ID3D11VertexShader> m_pVertexShader = nullptr; // 버텍스 셰이더
	ComPtr<ID3D11VertexShader> m_pSkyBoxVertexShader = nullptr;
	ComPtr<ID3D11PixelShader> m_pPixelShader = nullptr;   // 픽셀 셰이더
	ComPtr<ID3D11PixelShader> m_pSkyBoxPixelShader = nullptr;
	ComPtr<ID3D11InputLayout> m_pInputLayout = nullptr;   // 인풋 레이아웃
	ComPtr<ID3D11InputLayout> m_pSkyBoxInputLayout = nullptr;
	ComPtr<ID3D11Buffer> m_pVertexBuffer = nullptr;       // 버텍스 버퍼
	ComPtr<ID3D11Buffer> m_pIndexBuffer = nullptr;		  // 인덱스 버퍼
	ComPtr<ID3D11Buffer> m_pConstantBuffer = nullptr;	  // 상수 버퍼
	ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView = nullptr; // 텍스쳐를 입히기 위한 Shader Resource View
	ComPtr<ID3D11ShaderResourceView> m_pSkyBoxShaderResourceView = nullptr;
	ComPtr<ID3D11SamplerState> m_pSamplerState = nullptr; // Sampler State


	UINT m_VertexBufferStride = 0; // 버텍스 한개의 크기
	UINT m_VertexBufferOffset = 0; // 버텍스 한개에 대한 설정
	UINT m_VertexCount = 0; // 버텍스 개수
	int m_Indices = 0;

	Vector3 m_Scale1{ 1,1,1 };
	Vector3 m_Roataion1{ 0,0,0 };
	Vector3 m_Translation1{ 0,0,0 };
	
	Vector4 m_eye;
	Vector4 m_to;
	Vector4 m_up;

	Matrix m_WorldMatrix1;
	Matrix m_WorldMatrix2;
	Matrix m_WorldMatrix3;
	Matrix m_ViewMatrix; // 카메라 매트릭스
	Matrix m_ProjectionMatrix;

	Vector4 m_DirectionalLight[2];
	Vector4	m_LightColor[2];
	float	m_LightPower = 1.0f;
	

	float fovWidht;
	float fovHeight;
	float FieldOfView;
	float m_near = 0.1f;
	float m_far = 1000.0f;

	float elapsedTime = 0;

	bool Initialize() override;
	void LateInitialize() override;
	void Shutdown() override;
	//void MessageProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParameter, LPARAM lParameter) override;
	virtual void Render();
	
	void OnUpdate() override;

	bool InitD3D();
	void UnInitD3D();

	bool InitScene();
	void UnInitScene();

	void SetCube();
	void SetSkyBox();

	bool InitImGui();
	void UnInitImGui();

	void ImGuiBeginDraw();
	void ImGuiRender();
};


