#include "framework.h"
#include "DemoGameApp.h"
#include "../DirectX_3D_Lilbrary/Helper.h"
#include <d3dcompiler.h>
#include <DirectXTK/DDSTextureLoader.h>


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib") 
#pragma comment(lib, "d3dcompiler.lib")

//using namespace DirectX;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct Vertex
{
	Vector3 position;	
	Vector3 normal;
	Vector2 texture;

	Vertex(float x, float y,  float z) : position(x,y,z) { }
	Vertex(Vector3 position) : position(position) { }
	Vertex(Vector3 position, Vector3 normal) : position(position), normal(normal) { }
	Vertex(Vector3 position, Vector2 texture) : position(position), texture(texture){ normal = { 0,0,0 }; }
	Vertex(Vector3 position, Vector3 normal, Vector2 texture) : position(position), normal(normal), texture(texture) {  }
};

struct ConstantBuffer
{
	Matrix World; // 4
	Matrix View; // 4
	Matrix Projection; // 4

	Vector4 DirectionalLight[2];
	Vector4 DirectionalLightColor[2];
	Vector4 FinalColor;
};

DemoGameApp::DemoGameApp()
{
	
}

DemoGameApp::~DemoGameApp()
{

}

bool DemoGameApp::Initialize()
{
	__super::Initialize();

	if (!InitD3D()) return false;
	if (!InitScene()) return false;
	if (!InitImGui()) return false;

	return true;
}

void DemoGameApp::LateInitialize()
{

}

void DemoGameApp::Shutdown()
{
	UnInitImGui();
	UnInitScene();
	UnInitD3D();
	__super::Shutdown();
}

void DemoGameApp::OnUpdate()
{
	elapsedTime += TimeSystem::GetInstance()->deltaTime / 1000;
	
	DirectX::XMMATRIX translation1 = DirectX::XMMatrixTranslation(m_Translation1.x, m_Translation1.y, m_Translation1.z);
	DirectX::XMMATRIX rotation1 = DirectX::XMMatrixRotationRollPitchYaw(m_Roataion1.x, m_Roataion1.y, m_Roataion1.z);
	DirectX::XMMATRIX scale1 = DirectX::XMMatrixScaling(m_Scale1.x, m_Scale1.y, m_Scale1.z);
	m_WorldMatrix1 = scale1 * rotation1 * translation1;
	
	m_Camera.GetViewMatrix(m_ViewMatrix);
	m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(FieldOfView), (float)m_Width / m_Height, m_near, m_far);
}

LRESULT CALLBACK DemoGameApp::WndProc(HWND hWnd, UINT message, WPARAM wParameter, LPARAM lParameter)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParameter, lParameter))
	{ return true; }

	return __super::WndProc(hWnd, message, wParameter, lParameter);
}

void DemoGameApp::Render()
{
	float color[4] = { 0.0f, 0.5f, 0.5f, 1.0f };

	// 화면 해당 색으로 칠하기
	// 렌더타겟을 최종 출력 파이프라인에 바인딩
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilState.Get(), 1);
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), color);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0); // DepthStencilview 초기화, Depth버퍼 1.0f로 초기화
	
	// 정점 버퍼를 정해둔 버퍼세팅대로 정해둠
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정점을 이어서 그리는 방식
	m_pDeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &m_VertexBufferStride, &m_VertexBufferOffset);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout.Get());							// Input Layout설정
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);	// Index Buffer에 인덱스들 값 설정
	m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);					// VertexShader 설정
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());		// ConstantBuffer 초기화
	m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);					// PixelShader 설정
	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(0, 1, m_pShaderResourceView.GetAddressOf());
	m_pDeviceContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

	// 상수버퍼로 도형 생성
	ConstantBuffer cbuffer = {};
	cbuffer.World = XMMatrixTranspose(m_WorldMatrix1);
	cbuffer.View = XMMatrixTranspose(m_ViewMatrix);
	cbuffer.Projection = XMMatrixTranspose(m_ProjectionMatrix);
	cbuffer.DirectionalLight[0] = m_DirectionalLight[0];
	cbuffer.DirectionalLightColor[0] = m_LightColor[0];
	cbuffer.FinalColor = Vector4{ 0, 0, 0, 0 };
	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
	m_pDeviceContext->DrawIndexed(m_Indices, 0, 0);

	
	m_pDeviceContext->OMSetDepthStencilState(m_pSkyBoxDepthStencilState.Get(), 0);
	m_pDeviceContext->RSSetState(m_pRasterizerState.Get());
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정점을 이어서 그리는 방식
	m_pDeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &m_VertexBufferStride, &m_VertexBufferOffset);
	m_pDeviceContext->IASetInputLayout(m_pSkyBoxInputLayout.Get());
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);	// Index Buffer에 인덱스들 값 설정
	m_pDeviceContext->VSSetShader(m_pSkyBoxVertexShader.Get(), nullptr, 0);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());		// ConstantBuffer 초기화
	m_pDeviceContext->PSSetShader(m_pSkyBoxPixelShader.Get(), nullptr, 0);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(1, 1, m_pSkyBoxShaderResourceView.GetAddressOf());
	m_pDeviceContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());
	
	
	cbuffer.World = XMMatrixTranspose(DirectX::SimpleMath::Matrix::CreateTranslation(m_Camera.GetCameraPosition()));
 	cbuffer.View = XMMatrixTranspose(m_ViewMatrix);
	cbuffer.Projection = XMMatrixTranspose(m_ProjectionMatrix);
	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
	m_pDeviceContext->DrawIndexed(m_Indices, 0, 0);
	
	m_pDeviceContext->OMSetDepthStencilState(nullptr, 0);
	m_pDeviceContext->RSSetState(nullptr);

	//ImGUI 사용
	ImGuiBeginDraw();
	ImGui::Begin("Solar");
	ImGui::DragFloat3("Sun Position", &m_Translation1.x, 1.0f, -100.0f, 100.0f);
	ImGui::DragFloat3("Sun Rotation", &m_Roataion1.x, 1.0f, -360.0f, 360.0f);
	ImGui::DragFloat3("Sun Scale", &m_Scale1.x, 0.1f, 0.1f, 100.0f);

	ImGui::NewLine();
	ImGui::DragFloat3("Camera Position", &m_eye.x, 1.0f, -100.0f, 100.0f);

	ImGui::NewLine();
	ImGui::DragFloat3("Directional Light", &m_DirectionalLight[0].x, 0.01f, -1.0f, 1.0f);
	ImGui::ColorEdit4("Directional Light Color", &m_LightColor[0].x);
	ImGui::DragFloat("Directional Light Power", &m_LightPower, 0.1f, 0.0f, 10.0f);

	ImGui::NewLine();
	ImGui::DragFloat("Near", &m_near, 0.1f, 0.1f, m_far - 0.2f);
	ImGui::DragFloat("Far", &m_far, 0.1f, m_near + 0.2f, 1000.0f);
	ImGui::DragFloat("FoV", &FieldOfView, 0.1f, 0.1f, 360.0f);
	ImGui::End();
	ImGuiRender();

	m_pSwapChain->Present(0, 0); // 실제 모니터로 출력
}

bool DemoGameApp::InitD3D()
{
	HRESULT hr = 0;

	//D3D Device 생성

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creationFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif
	
	// 그래픽 카드 하드웨어의 스펙으로 호환되는 가장 높은 DirectX 기능레벨로 생성하여 드라이버가 작동
	// 인터페이스는 D3D11이지만 GPU 드라이버는 D3D12 드라이버가 작동할 수 있음

	D3D_FEATURE_LEVEL featureLevels[] = {
		//인덱스 0부터 차례대로 시도
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0 
	};

	D3D_FEATURE_LEVEL actualFeaturelevel = {};
	
	D3D11CreateDevice(
		nullptr, 
		D3D_DRIVER_TYPE_HARDWARE, 
		nullptr,
		creationFlags, 
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION, 
		m_pDevice.GetAddressOf(), 
		&actualFeaturelevel, 
		m_pDeviceContext.GetAddressOf()
	);
	UINT dxgifactoryFlags = 0;
#ifdef _DEBUG
	dxgifactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif // _DEBUG

	ComPtr<IDXGIFactory7> dxgiFactory;
	CreateDXGIFactory2(dxgifactoryFlags, IID_PPV_ARGS(&dxgiFactory));
	
	//SwapChain 생성
	//IDXGIFACTORY7을 사용해서 SWAPCHAIN만드려면 DXGI_SWAP_CHAIN_DESC1 구조체가 필요
	DXGI_SWAP_CHAIN_DESC1 swapdesc = {};
	swapdesc.BufferCount = 2;
	swapdesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapdesc.Width = m_Width; // 백버퍼의 가로 / 세로 크기 설정
	swapdesc.Height = m_Height;
	swapdesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapdesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapdesc.Stereo = false;
	// 샘플링 관련 설정
	swapdesc.SampleDesc.Count = 1;
	swapdesc.SampleDesc.Quality = 0;

	// 화면 늘리기 설정
	swapdesc.Scaling = DXGI_SCALING_NONE;				// 화면 크기대로 늘리기
	//swapdesc.Scaling = DXGI_SCALING_ASPECT_RATIO_STRETCH; // 스왑체인의 백버퍼 크기가 조정될 경우 가로세로 비율을 유지하고 남는 부분은 검정 테두리

	hr = dxgiFactory->CreateSwapChainForHwnd(
		m_pDevice.Get(),
		m_handleWindow,
		&swapdesc,
		nullptr,
		nullptr,
		m_pSwapChain.GetAddressOf());


	//RenderTargetView 생성
	ComPtr<ID3D11Texture2D> BackBuffer;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)BackBuffer.GetAddressOf()));
	m_pDevice->CreateRenderTargetView(BackBuffer.Get(), nullptr, m_pRenderTargetView.GetAddressOf());

	// 뷰포트 설정
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)m_Width;
	viewport.Height = (float)m_Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &viewport);
	 
	// Depth / Stencil View 생성
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = m_Width;
	depthDesc.Height = m_Height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> pTextureDepthStencil;
	HR_T(m_pDevice->CreateTexture2D(&depthDesc, nullptr, pTextureDepthStencil.GetAddressOf()));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {}; // 초기화 잊지 않기...!!
	depthStencilViewDesc.Format = depthDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	HR_T(m_pDevice->CreateDepthStencilView(pTextureDepthStencil.Get(), &depthStencilViewDesc, m_pDepthStencilView.GetAddressOf()));

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true; // Depth Test 활성화
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; // 깊이 버퍼 업데이트 허용
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS; // Z값이 낮으면 앞으로 렌더링
	depthStencilDesc.StencilEnable = false; // Stencil Test 비활성화

	//ComPtr<ID3D11DepthStencilState> depthStencilState = nullptr;
	m_pDevice->CreateDepthStencilState(&depthStencilDesc, m_pDepthStencilState.GetAddressOf());

	// skybox 전용 depth/stencil state 생성
	D3D11_DEPTH_STENCIL_DESC skyDSD = {};
	skyDSD.DepthEnable = true;
	skyDSD.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	skyDSD.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // 깊이데이터를 기존 깊이 데이터와 비교, 원본 데이터가 대상데이터보다 작거나 같으면 비교 통과
	m_pDevice->CreateDepthStencilState(&skyDSD, m_pSkyBoxDepthStencilState.GetAddressOf());

	// Rasterizer State
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthClipEnable = true;
	m_pDevice->CreateRasterizerState(&rasterizerDesc, m_pRasterizerState.GetAddressOf());

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;

	D3D11_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = true;
	renderTargetBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;		 // Src의 Alpha값
	renderTargetBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // DestBlend는 (1 - SrcColor.a)
	// FinalAlpha = (SrcAlpha * SrcBlendAlpha) + (DestAlpha * DestBlendAlpha)
	renderTargetBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;		 // SrcBlendAlpha = 1
	renderTargetBlendDesc.DestBlendAlpha = D3D11_BLEND_ONE;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0] = renderTargetBlendDesc;
	HR_T(m_pDevice->CreateBlendState(&blendDesc, m_pAlphaBlendState.GetAddressOf()));

	return true;
}

void DemoGameApp::UnInitD3D()
{
	
}

bool DemoGameApp::InitScene()
{
	// 성공여부 판단하는 변수
	SetCube();
	//SetSkyBox();
	

	return true;
}

void DemoGameApp::UnInitScene()
{
	
}

void DemoGameApp::SetCube()
{
	HRESULT hr = 0;

	ID3D10Blob* errorMessage = nullptr;

	Vertex vertices[] =
	{
		//라이팅 큐브
			//노말벡터가 Y+방향
			Vertex{Vector3{-1.0f, 1.0f, -1.0f},  Vector3{0.0f,  1.0f, 0.0f}, Vector2{0.0f, 1.0f}}, // 0
			Vertex{Vector3{-1.0f, 1.0f,  1.0f},  Vector3{0.0f,  1.0f, 0.0f}, Vector2{0.0f, 0.0f}}, // 1
			Vertex{Vector3{ 1.0f, 1.0f,  1.0f},  Vector3{0.0f,  1.0f, 0.0f}, Vector2{1.0f, 0.0f}}, // 2
			Vertex{Vector3{ 1.0f, 1.0f, -1.0f},  Vector3{0.0f,  1.0f, 0.0f}, Vector2{1.0f, 1.0f}}, // 3

			//노말벡터가 Y-방향
			Vertex{Vector3{-1.0f, -1.0f,  1.0f}, Vector3{0.0f, -1.0f, 0.0f}, Vector2{0.0f, 1.0f}}, // 4
			Vertex{Vector3{-1.0f, -1.0f, -1.0f}, Vector3{0.0f, -1.0f, 0.0f}, Vector2{0.0f, 0.0f}}, // 5
			Vertex{Vector3{ 1.0f, -1.0f, -1.0f}, Vector3{0.0f, -1.0f, 0.0f}, Vector2{1.0f, 0.0f}}, // 6
			Vertex{Vector3{ 1.0f, -1.0f,  1.0f}, Vector3{0.0f, -1.0f, 0.0f}, Vector2{1.0f, 1.0f}}, // 7

			//노말벡터가 X- 방향
			Vertex{Vector3{-1.0f, -1.0f,  1.0f}, Vector3{-1.0f, 0.0f, 0.0f}, Vector2{0.0f, 1.0f}}, // 8
			Vertex{Vector3{-1.0f,  1.0f,  1.0f}, Vector3{-1.0f, 0.0f, 0.0f}, Vector2{0.0f, 0.0f}}, // 9
			Vertex{Vector3{-1.0f,  1.0f, -1.0f}, Vector3{-1.0f, 0.0f, 0.0f}, Vector2{1.0f, 0.0f}}, // 10
			Vertex{Vector3{-1.0f, -1.0f, -1.0f}, Vector3{-1.0f, 0.0f, 0.0f}, Vector2{1.0f, 1.0f}}, // 11

			//노말벡터가 X+ 방향
			Vertex{Vector3{ 1.0f, -1.0f, -1.0f}, Vector3{ 1.0f, 0.0f, 0.0f}, Vector2{0.0f, 1.0f}}, // 12
			Vertex{Vector3{ 1.0f,  1.0f, -1.0f}, Vector3{ 1.0f, 0.0f, 0.0f}, Vector2{0.0f, 0.0f}}, // 13
			Vertex{Vector3{ 1.0f,  1.0f,  1.0f}, Vector3{ 1.0f, 0.0f, 0.0f}, Vector2{1.0f, 0.0f}}, // 14
			Vertex{Vector3{ 1.0f, -1.0f,  1.0f}, Vector3{ 1.0f, 0.0f, 0.0f}, Vector2{1.0f, 1.0f}}, // 15

			//노말벡터가 Z- 방향
			Vertex{Vector3{-1.0f, -1.0f, -1.0f}, Vector3{0.0f, 0.0f, -1.0f}, Vector2{0.0f, 1.0f}}, // 16
			Vertex{Vector3{-1.0f,  1.0f, -1.0f}, Vector3{0.0f, 0.0f, -1.0f}, Vector2{0.0f, 0.0f}}, // 17
			Vertex{Vector3{ 1.0f,  1.0f, -1.0f}, Vector3{0.0f, 0.0f, -1.0f}, Vector2{1.0f, 0.0f}}, // 18
			Vertex{Vector3{ 1.0f, -1.0f, -1.0f}, Vector3{0.0f, 0.0f, -1.0f}, Vector2{1.0f, 1.0f}}, // 19

			//노말벡터가 Z+방향
			Vertex{Vector3{ 1.0f, -1.0f,  1.0f}, Vector3{0.0f, 0.0f,  1.0f}, Vector2{0.0f, 1.0f}}, // 20
			Vertex{Vector3{ 1.0f,  1.0f,  1.0f}, Vector3{0.0f, 0.0f,  1.0f}, Vector2{0.0f, 0.0f}}, // 21
			Vertex{Vector3{-1.0f,  1.0f,  1.0f}, Vector3{0.0f, 0.0f,  1.0f}, Vector2{1.0f, 0.0f}}, // 22
			Vertex{Vector3{-1.0f, -1.0f,  1.0f}, Vector3{0.0f, 0.0f,  1.0f}, Vector2{1.0f, 1.0f}}, // 23
	};

	// 정점 버퍼 설정
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	m_VertexCount = ARRAYSIZE(vertices);
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * m_VertexCount;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	// 정점 버퍼 생성
	D3D11_SUBRESOURCE_DATA vertexBufferData = {};
	vertexBufferData.pSysMem = vertices; // 버텍스 정보 입력

	// 버텍스 버퍼 설정, 버텍스 버퍼 데이터를 가지고 버텍스 버퍼 생성
	HR_T(hr = m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, m_pVertexBuffer.GetAddressOf()));

	// 버텍스 버퍼 정보
	m_VertexBufferStride = sizeof(Vertex);
	m_VertexBufferOffset = 0;

	// Render에서 파이프라인에 바인딩할 버텍스 셰이더 생성
	ComPtr<ID3DBlob> vertexShaderBufffer = nullptr; // 버텍스 셰이더 HLSL의 컴파일된 결과를 담을 수 있는 버퍼 객체
	// 컴파일할 셰이더 파일의 이름과 함수, 버전 선택
	HR_T(CompileShaderFromFile(L"VertexShader.hlsl", "main", "vs_4_0", vertexShaderBufffer.GetAddressOf()));
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBufffer->GetBufferPointer(), // 
		vertexShaderBufffer->GetBufferSize(), NULL, m_pVertexShader.GetAddressOf()));

	ComPtr<ID3DBlob> skyboxVertexShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"SkyboxVertexShader.hlsl", "main", "vs_4_0", skyboxVertexShaderBuffer.GetAddressOf()));
	HR_T(m_pDevice->CreateVertexShader(skyboxVertexShaderBuffer->GetBufferPointer(),
		skyboxVertexShaderBuffer->GetBufferSize(), NULL, m_pSkyBoxVertexShader.GetAddressOf()));

	//InputLayout 생성
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HR_T(m_pDevice->CreateInputLayout(
		layout,
		ARRAYSIZE(layout),
		vertexShaderBufffer->GetBufferPointer(),
		vertexShaderBufffer->GetBufferSize(),
		m_pInputLayout.GetAddressOf()));

	D3D11_INPUT_ELEMENT_DESC texturecubeLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HR_T(m_pDevice->CreateInputLayout(
		texturecubeLayout,
		ARRAYSIZE(texturecubeLayout),
		skyboxVertexShaderBuffer->GetBufferPointer(),
		skyboxVertexShaderBuffer->GetBufferSize(),
		m_pSkyBoxInputLayout.GetAddressOf()));

	//그림을 그릴 점들의 인덱스 설정
	unsigned int indices[] =
	{
		// 일반 큐브
			//윗변
			0,1,2, 0,2,3,

			//밑변
			4,5,6, 4,6,7,

			//왼쪽 변
			8,9,10,	8,10,11,

			//오른쪽 변
			12,13,14, 12,14,15,

			//정면
			16,17,18, 16,18,19,

			//뒷면
			20,21,22, 20,22,23,

		// 스카이박스
			//위
			//24,25,26, 24,26,27,

			////아래
			//28,29,30, 28,30,31,

			////왼쪽
			//32,33,34, 32,34,35,

			////오른쪽
			//36,37,38, 36,38,39,

			////뒤
			//40,41,42, 40,42,43,

			////앞
			//44,45,46, 44,46,47
	};
	m_Indices = ARRAYSIZE(indices);

	// 인덱스 버퍼 생성
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.ByteWidth = sizeof(unsigned int) * ARRAYSIZE(indices);
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices;
	HR_T(m_pDevice->CreateBuffer(&ibDesc, &ibData, m_pIndexBuffer.GetAddressOf()));

	// 픽셀 셰이더 생성
	ComPtr<ID3DBlob> pixelShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"PixelShader.hlsl", "main", "ps_4_0", pixelShaderBuffer.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, m_pPixelShader.GetAddressOf()));

	// 스카이박스 전용 픽셀셰이더 생성
	ComPtr<ID3DBlob> skyboxPixelShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"SkyBoxPixelShader.hlsl", "main", "ps_4_0", skyboxPixelShaderBuffer.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(
		skyboxPixelShaderBuffer->GetBufferPointer(),
		skyboxPixelShaderBuffer->GetBufferSize(), NULL, m_pSkyBoxPixelShader.GetAddressOf()));

	// 상수 버퍼 생성
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT; // 기본으로 설정
	cbDesc.ByteWidth = sizeof(ConstantBuffer); // 사이즈 설정
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // 상수버퍼로 바인딩
	cbDesc.CPUAccessFlags = 0; // cpu에 대한 엑세스가 필요한지 여부 -> 현재는 필요없으니 0으로 설정
	HR_T(m_pDevice->CreateBuffer(&cbDesc, nullptr, m_pConstantBuffer.GetAddressOf())); // 버퍼 생성
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf()); // 상수버퍼를 Vertex Shader에 연결

	ComPtr<ID3D11Resource> sampletexture = nullptr;
	HR_T(DirectX::CreateDDSTextureFromFile(m_pDevice.Get(), L"../Resources/SampleTexture.dds", sampletexture.GetAddressOf(), m_pShaderResourceView.GetAddressOf()));

	ComPtr<ID3D11Resource> skycubeTexture = nullptr;
	HR_T(DirectX::CreateDDSTextureFromFile(m_pDevice.Get(), L"../Resources/cubemap.dds", skycubeTexture.GetAddressOf(), m_pSkyBoxShaderResourceView.GetAddressOf()));

	// SamplerState 생성
	D3D11_SAMPLER_DESC samplerStateDesc = {};
	samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerStateDesc.MinLOD = 0;
	samplerStateDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR_T(m_pDevice->CreateSamplerState(&samplerStateDesc, m_pSamplerState.GetAddressOf()));

	// 상수버퍼에 쓸 월드 행렬 초기화
	m_WorldMatrix1 = DirectX::XMMatrixIdentity();

	//카메라의 벡터 구하기
	m_eye = { 0.0f, 1.0f, -5.0f, 0.0f };
	m_to = { 0.0f, 0.0f, 1.0f, 0.0f };
	m_up = { 0.0f, 1.0f, 0.0f, 0.0f };

	DirectX::XMVECTOR eye = DirectX::XMVectorSet(m_eye.x, m_eye.y, m_eye.z, m_eye.w); // 카메라의 위치
	DirectX::XMVECTOR to = DirectX::XMVectorSet(m_to.x, m_to.y, m_to.z, m_to.w);   // 카메라가 바라보는 방향
	DirectX::XMVECTOR up = DirectX::XMVectorSet(m_up.x, m_up.y, m_up.z, m_up.w);   // 카메라의 Up벡터

	//m_ViewMatrix = DirectX::XMMatrixLookToLH(eye, to, up); // 세 벡터를 가지고 카메라의 행렬 만듦

	fovWidht = m_Width;
	fovHeight = m_Height;
	FieldOfView = 90;

	m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(FieldOfView), (float)m_Width / m_Height, m_near, m_far);

	m_DirectionalLight[0] = Vector4{ 0.0f, 0.0f, -1.0f, 1.0f };
	m_LightColor[0] = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void DemoGameApp::SetSkyBox()
{
	HRESULT hr;

	ID3DBlob* errormessage = nullptr;

	Vertex skyboxVertices[] =
	{
	//스카이박스
		//Y+방향
		Vertex{Vector3{-1.0f, 1.0f,  1.0f}}, // 0
		Vertex{Vector3{-1.0f, 1.0f, -1.0f}}, // 1
		Vertex{Vector3{ 1.0f, 1.0f, -1.0f}}, // 2
		Vertex{Vector3{ 1.0f, 1.0f,  1.0f}}, // 3 

		//Y-방향
		Vertex{Vector3{-1.0f, -1.0f, -1.0f}}, // 4
		Vertex{Vector3{-1.0f, -1.0f,  1.0f}}, // 5
		Vertex{Vector3{ 1.0f, -1.0f,  1.0f}}, // 6
		Vertex{Vector3{ 1.0f, -1.0f, -1.0f}}, // 7

		//X- 방향
		Vertex{Vector3{-1.0f,  1.0f,  1.0f}}, // 8
		Vertex{Vector3{-1.0f, -1.0f,  1.0f}}, // 9
		Vertex{Vector3{-1.0f, -1.0f, -1.0f}}, // 10
		Vertex{Vector3{-1.0f,  1.0f, -1.0f}}, // 11

		//X+ 방향
		Vertex{Vector3{ 1.0f,  1.0f, -1.0f}}, // 12
		Vertex{Vector3{ 1.0f, -1.0f, -1.0f}}, // 13
		Vertex{Vector3{ 1.0f, -1.0f,  1.0f}}, // 14
		Vertex{Vector3{ 1.0f,  1.0f,  1.0f}}, // 15

		//Z- 방향
		Vertex{Vector3{-1.0f,  1.0f, -1.0f}}, // 16
		Vertex{Vector3{-1.0f, -1.0f, -1.0f}}, // 17
		Vertex{Vector3{ 1.0f, -1.0f, -1.0f}}, // 18
		Vertex{Vector3{ 1.0f,  1.0f, -1.0f}}, // 19

		//Z+방향
		Vertex{Vector3{ 1.0f,  1.0f,  1.0f}}, // 20
		Vertex{Vector3{ 1.0f, -1.0f,  1.0f}}, // 21
		Vertex{Vector3{-1.0f, -1.0f,  1.0f}}, // 22
		Vertex{Vector3{-1.0f,  1.0f,  1.0f}}, // 23
	};


}

bool DemoGameApp::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark(); // 색상
	
	ImGui_ImplWin32_Init(GameApp::m_handleWindow);
	ImGui_ImplDX11_Init(m_pDevice.Get(), m_pDeviceContext.Get());

	return true;
}

void DemoGameApp::UnInitImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void DemoGameApp::ImGuiBeginDraw()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void DemoGameApp::ImGuiRender()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
