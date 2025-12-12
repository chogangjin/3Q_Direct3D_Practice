#pragma once
#include "../DirectX_3D_Lilbrary/GameApp.h"
#include <d3d11.h>
#include <directxtk/Simplemath.h>

#include <imgui.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "ModelLoader.h"
#include "Mesh.h"

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
	ComPtr<ID3D11DepthStencilView> m_pShadowMapDepthStencilView = nullptr;
	ComPtr<ID3D11RasterizerState> m_pRasterizerState = nullptr;
	ComPtr<ID3D11BlendState> m_pAlphaBlendState = nullptr;

	//렌더링 파이프라인에 사용되는 정보들 
	ComPtr<ID3D11VertexShader> m_pVertexShader = nullptr; // 버텍스 셰이더
	ComPtr<ID3D11VertexShader> m_pSkyBoxVertexShader = nullptr;
	ComPtr<ID3D11VertexShader> m_pShadowVertexBuffer = nullptr;
	ComPtr<ID3D11PixelShader> m_pPixelShader = nullptr;   // 픽셀 셰이더
	ComPtr<ID3D11PixelShader> m_pSkyBoxPixelShader = nullptr;
	ComPtr<ID3D11InputLayout> m_pInputLayout = nullptr;   // 인풋 레이아웃
	ComPtr<ID3D11InputLayout> m_pSkyBoxInputLayout = nullptr;
	ComPtr<ID3D11Buffer> m_pVertexBuffer = nullptr;       // 버텍스 버퍼
	ComPtr<ID3D11Buffer> m_pIndexBuffer = nullptr;		  // 인덱스 버퍼

	ComPtr<ID3D11Buffer> m_pConstantBuffer = nullptr;	  // 상수 버퍼
	ComPtr<ID3D11Buffer> m_pBonePoseBuffer = nullptr;
	ComPtr<ID3D11Buffer> m_pBoneOffsetBuffer = nullptr;
	ComPtr<ID3D11Buffer> m_pShadowBuffer = nullptr;
	ComPtr<ID3D11Buffer> m_pLightBuffer = nullptr;
	ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView = nullptr; // 텍스쳐를 입히기 위한 Shader Resource View
	ComPtr<ID3D11ShaderResourceView> m_pDefaultShaderResourceView = nullptr;
	ComPtr<ID3D11ShaderResourceView> m_pNormalMap = nullptr;
	ComPtr<ID3D11ShaderResourceView> m_pSpecularMap = nullptr;
	ComPtr<ID3D11ShaderResourceView> m_pSkyBoxShaderResourceView = nullptr;
	ComPtr<ID3D11ShaderResourceView> m_pShadowMapShaderResourceView = nullptr;
	ComPtr<ID3D11SamplerState> m_pSamplerState = nullptr; // Sampler State
	UINT m_VertexBufferStride = 0; // 버텍스 한개의 크기
	UINT m_VertexBufferOffset = 0; // 버텍스 한개에 대한 설정
	UINT m_VertexCount = 0; // 버텍스 개수
	int m_Indices = 0;
	D3D11_VIEWPORT m_Viewport = {};

	// 공간별 행렬
	Matrix m_WorldMatrix = DirectX::XMMatrixIdentity();
	Matrix m_ViewMatrix; // 카메라 매트릭스
	Matrix m_ProjectionMatrix;

	// 그림자 행렬

	// 그림자 관련 멤버변수
	D3D11_VIEWPORT m_ShadowViewport;
	Vector3 m_ShadowPos  = Vector3::Zero;
	Vector3 m_ShadowLookAt = Vector3::Zero;
	Matrix m_LightView = DirectX::XMMatrixIdentity();
	Matrix m_LightProjection = DirectX::XMMatrixIdentity();
	ComPtr<ID3D11Texture2D> m_pShadowMap = nullptr; // 그림자 맵핑을

	Matrix  m_ShadowView;
	Matrix  m_ShadowProjection;
	//그림자 fov 설정
	Vector2 m_ShadowProjectionNearFar{ 100.0f, 100000.0f };
	float m_ShadowFow = 50.0f;
	float m_ShadowForwardDistanceFromCamera = 300.0f;
	float m_ShadowUpDistanceFromLookAt = 100;

	Vector3 m_DirectionalLight = Vector3{ 0.0f, -1.0f, 1.0f};
	Vector4 m_DiffuseColor{ 0.9f,0.9f,0.9f,0.9f };
	Vector4 m_DiffuseMaterial{ 0.9f,0.9f,0.9f,1.0f };
	Vector4 m_AmbientColor{ 0.1f,0.1f,0.1f,1.0f };
	Vector4 m_AmbientMaterial{ 0.1f,0.1f,0.1f,1.0f };
	Vector4 m_SpecularColor{ 0.9f,0.9f,0.9f,1.0f };
	Vector4 m_SpecularMaterial{ 0.9f,0.9f,0.9f,1.0f };
	Vector4	m_LightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	float	m_Shininess = 1.0f;

	float fovWidht=0;
	float fovHeight = 0;
	float FieldOfView = 0;
	float m_near = 0.1f;
	float m_far = 10000.0f;

	float elapsedTime = 0;

	SkeletalMesh m_SkinningModel;
	SkeletalMesh m_Robot;
	SkeletalMesh m_Plain;

	bool m_bDebugShadow = true;

	bool Initialize() override;
	void LateInitialize() override;
	void Shutdown() override;
	
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParameter, LPARAM lParameter) override;
	virtual void Render();
	
	void OnUpdate() override;

	bool InitD3D();
	void UnInitD3D();

	bool InitScene();
	void UnInitScene();

	void SetCube();

	bool InitImGui();
	void UnInitImGui();

	void ImGuiBeginDraw();
	void ImGuiRender();
	void ImGuiEndDraw();
};


