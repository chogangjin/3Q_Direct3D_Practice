#include "framework.h"
#include <array>

#include "DemoGameApp.h"
#include "SkyBoxVertex.h"
#include "../DirectX_3D_Lilbrary/Helper.h"
#include <d3dcompiler.h>
#include <DirectXTK/DDSTextureLoader.h>
#include <directxtk/WICTextureLoader.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib") 
#pragma comment(lib, "d3dcompiler.lib")

//using namespace DirectX;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);




struct ConstantBuffer
{
	Matrix World;		// 16
	Matrix View;		// 16
	Matrix Projection;  // 16

	Vector4 DirectionalLight;
	//float padding;

	Vector4 DirectionalLightColor;
	
	Vector4 DiffuseColor;
	Vector4 AmbientColor;
	Vector4 SpecularColor;
	
	Vector4 DiffuseMaterial;
	Vector4 AmbientMaterial;
	Vector4 SpecularMaterial;

	Vector3 CameraPos;
	float shininess;
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
	
	DirectX::XMMATRIX translation1 = DirectX::XMMatrixTranslation(m_TranslationTree.x, m_TranslationTree.y, m_TranslationTree.z);
	DirectX::XMMATRIX rotation1 = DirectX::XMMatrixRotationRollPitchYaw(m_RoataionTree.x, m_RoataionTree.y, m_RoataionTree.z);
	DirectX::XMMATRIX scale1 = DirectX::XMMatrixScaling(m_ScaleTree.x, m_ScaleTree.y, m_ScaleTree.z);
	m_WorldMatrix = scale1 * rotation1 * translation1;
	
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
	
	// 상수버퍼로 도형 생성
	ConstantBuffer cbuffer = {};
	
	cbuffer.View = XMMatrixTranspose(m_ViewMatrix);
	cbuffer.Projection = XMMatrixTranspose(m_ProjectionMatrix);
	cbuffer.DirectionalLight = Vector4{ m_DirectionalLight };
	cbuffer.DirectionalLightColor = m_LightColor;
	cbuffer.DiffuseColor = m_DiffuseColor;
	cbuffer.DiffuseMaterial = m_DiffuseMaterial;
	cbuffer.AmbientColor = m_AmbientColor;
	cbuffer.AmbientMaterial = m_AmbientMaterial;
	cbuffer.SpecularColor = m_SpecularColor;
	cbuffer.SpecularMaterial = m_SpecularMaterial;
	cbuffer.CameraPos = m_Camera.GetCameraPosition();
	cbuffer.shininess = m_Shininess;

	// 화면 해당 색으로 칠하기
	// 렌더타겟을 최종 출력 파이프라인에 바인딩
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), color);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0); // DepthStencilview 초기화, Depth버퍼 1.0f로 초기화
	
	// 정점 버퍼를 정해둔 버퍼세팅대로 정해둠
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정점을 이어서 그리는 방식
	m_pDeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &m_VertexBufferStride, &m_VertexBufferOffset);
	m_pDeviceContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

	//skybox
	m_pDeviceContext->OMSetDepthStencilState(m_pSkyBoxDepthStencilState.Get(), 0);
	m_pDeviceContext->RSSetState(m_pRasterizerState.Get());
	m_pDeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &m_VertexBufferStride, &m_VertexBufferOffset);
	m_pDeviceContext->IASetInputLayout(m_pSkyBoxInputLayout.Get());
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);	// Index Buffer에 인덱스들 값 설정
	m_pDeviceContext->VSSetShader(m_pSkyBoxVertexShader.Get(), nullptr, 0);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());		// ConstantBuffer 초기화
	m_pDeviceContext->PSSetShader(m_pSkyBoxPixelShader.Get(), nullptr, 0);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(1, 1, m_pSkyBoxShaderResourceView.GetAddressOf());
	
	cbuffer.World = XMMatrixTranspose(DirectX::SimpleMath::Matrix::CreateTranslation(m_Camera.GetCameraPosition()));
 	
	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
	m_pDeviceContext->DrawIndexed(m_Indices, 0, 0);
	m_pDeviceContext->OMSetDepthStencilState(nullptr, 0);
	m_pDeviceContext->RSSetState(nullptr);

	// fbx
	cbuffer.World = XMMatrixTranspose(m_WorldMatrix);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout.Get());							// Input Layout설정
	m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);					// VertexShader 설정
	m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);					// PixelShader 설정
	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
	m_pDeviceContext->OMSetBlendState(m_pAlphaBlendState.Get(), nullptr , 0xffffffff);
	
	m_WorldMatrix = DirectX::XMMatrixScaling(m_ScaleZelda.x, m_ScaleZelda.y, m_ScaleZelda.z) * DirectX::XMMatrixRotationRollPitchYaw(m_RoataionZelda.x, m_RoataionZelda.y, m_RoataionZelda.z) * DirectX::XMMatrixTranslation(m_TranslationZelda.x, m_TranslationZelda.y, m_TranslationZelda.z);
	cbuffer.World = XMMatrixTranspose(m_WorldMatrix);
	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
	m_ZeldaModel.Draw(m_pDeviceContext.Get());

	m_WorldMatrix = DirectX::XMMatrixScaling(m_ScaleTree.x, m_ScaleTree.y, m_ScaleTree.z) * DirectX::XMMatrixRotationRollPitchYaw(m_RoataionTree.x, m_RoataionTree.y, m_RoataionTree.z) * DirectX::XMMatrixTranslation(m_TranslationTree.x, m_TranslationTree.y, m_TranslationTree.z);
	cbuffer.World = XMMatrixTranspose(m_WorldMatrix);
	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
	m_TreeModel.Draw(m_pDeviceContext.Get());



	m_WorldMatrix = DirectX::XMMatrixScaling(m_ScaleCharacter.x, m_ScaleCharacter.y, m_ScaleCharacter.z) * DirectX::XMMatrixRotationRollPitchYaw(m_RotationCharacter.x, m_RotationCharacter.y, m_RotationCharacter.z) * XMMatrixTranslation(m_TranslationCharacter.x, m_TranslationCharacter.y, m_TranslationCharacter.z);
	cbuffer.World = XMMatrixTranspose(m_WorldMatrix);
	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
	m_Character.Draw(m_pDeviceContext.Get());
	m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	

	//ImGUI 사용
	ImGuiBeginDraw();
	ImGui::Begin("Solar");
	ImGui::SeparatorText("Tree");
	ImGui::DragFloat3("Tree Position",  &m_TranslationTree.x, 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat3("Tree Rotation",  &m_RoataionTree.x, 0.01f, -100.0f, 100.0f);
	ImGui::DragFloat3("Tree Scale",		&m_ScaleTree.x, 0.1f, 0.1f, 100.0f);

	ImGui::SeparatorText("Zelda");
	ImGui::DragFloat3("Zelda Position",   &m_TranslationZelda.x, 1.0f, -100.0f, 100.0f);
	ImGui::DragFloat3("Zelda Rotation",   &m_RoataionZelda.x,	 0.01f, -360.0f, 360.0f);
	ImGui::DragFloat3("Zelda Scale",	  &m_ScaleZelda.x,		 0.1f, 0.1f, 100.0f);

	ImGui::SeparatorText("Character");
	ImGui::DragFloat3("Character Position", &m_TranslationCharacter.x, 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat3("Character Rotation", &m_RotationCharacter.x, 0.01f, -100.0f, 100.0f);
	ImGui::DragFloat3("Character Scale", &m_ScaleCharacter.x, 1.0f, 0.1f, 100.0f);

	ImGui::NewLine();
	ImGui::SeparatorText("Light");
	ImGui::DragFloat3("Directional Light",  &m_DirectionalLight.x, 0.01f, -1.0f, 1.0f);
	ImGui::ColorEdit4("Diffuse Color",		&m_LightColor.x);
	ImGui::ColorEdit4("Ambient Color",		&m_AmbientColor.x);
	ImGui::ColorEdit4("SpecularColor",		&m_SpecularColor.x);

	ImGui::NewLine();
	ImGui::SeparatorText("Material");
	ImGui::ColorEdit4("Diffuse Material", &m_DiffuseMaterial.x);
	ImGui::ColorEdit4("Ambient Material", &m_AmbientMaterial.x);
	ImGui::ColorEdit4("Specular Material", &m_SpecularMaterial.x);
	ImGui::DragFloat("Shininess", &m_Shininess, 0.1f, 0.0f, 10000.0f);

	ImGui::NewLine();
	ImGui::SeparatorText("Camera Setting");
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
	blendDesc.IndependentBlendEnable = true;

	D3D11_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;		 // Src의 Alpha값
	renderTargetBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // DestBlend는 (1 - SrcColor.a)
	// FinalAlpha = (SrcAlpha * SrcBlendAlpha) + (DestAlpha * DestBlendAlpha)
	renderTargetBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;		 // SrcBlendAlpha = 1
	renderTargetBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
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

	return true;
}

void DemoGameApp::UnInitScene()
{
	
}

void DemoGameApp::SetCube()
{
	HRESULT hr = 0;
	
	ID3D10Blob* errorMessage = nullptr;

	SkyBoxVertex vertices[] =
	{
		//라이팅 큐브
			//노말벡터가 Y+방향
			//                   position                texture         
			SkyBoxVertex{Vector3{-1.0f, 1.0f, -1.0f}, Vector2{0.0f, 1.0f}}, // 0
			SkyBoxVertex{Vector3{-1.0f, 1.0f,  1.0f}, Vector2{0.0f, 0.0f}}, // 1
			SkyBoxVertex{Vector3{ 1.0f, 1.0f,  1.0f}, Vector2{1.0f, 0.0f}}, // 2
			SkyBoxVertex{Vector3{ 1.0f, 1.0f, -1.0f}, Vector2{1.0f, 1.0f}}, // 3
												   
			//Normal벡터가 Y-방향										 
			SkyBoxVertex{Vector3{-1.0f, -1.0f,  1.0f}, Vector2{0.0f, 1.0f}}, // 4
			SkyBoxVertex{Vector3{-1.0f, -1.0f, -1.0f}, Vector2{0.0f, 0.0f}}, // 5
			SkyBoxVertex{Vector3{ 1.0f, -1.0f, -1.0f}, Vector2{1.0f, 0.0f}}, // 6
			SkyBoxVertex{Vector3{ 1.0f, -1.0f,  1.0f}, Vector2{1.0f, 1.0f}}, // 7
			
			//Normal벡터가 X- 방향										 
			SkyBoxVertex{Vector3{-1.0f, -1.0f,  1.0f}, Vector2{0.0f, 1.0f}}, // 8
			SkyBoxVertex{Vector3{-1.0f,  1.0f,  1.0f}, Vector2{0.0f, 0.0f}}, // 9
			SkyBoxVertex{Vector3{-1.0f,  1.0f, -1.0f}, Vector2{1.0f, 0.0f}}, // 10
			SkyBoxVertex{Vector3{-1.0f, -1.0f, -1.0f}, Vector2{1.0f, 1.0f}}, // 11
			
			//Normal벡터가 X+ 방향										 
			SkyBoxVertex{Vector3{ 1.0f, -1.0f, -1.0f}, Vector2{0.0f, 1.0f}}, // 12
			SkyBoxVertex{Vector3{ 1.0f,  1.0f, -1.0f}, Vector2{0.0f, 0.0f}}, // 13
			SkyBoxVertex{Vector3{ 1.0f,  1.0f,  1.0f}, Vector2{1.0f, 0.0f}}, // 14
			SkyBoxVertex{Vector3{ 1.0f, -1.0f,  1.0f}, Vector2{1.0f, 1.0f}}, // 15
			
			//Normal벡터가 Z- 방향										 
			SkyBoxVertex{Vector3{-1.0f, -1.0f, -1.0f}, Vector2{0.0f, 1.0f}}, // 16
			SkyBoxVertex{Vector3{-1.0f,  1.0f, -1.0f}, Vector2{0.0f, 0.0f}}, // 17
			SkyBoxVertex{Vector3{ 1.0f,  1.0f, -1.0f}, Vector2{1.0f, 0.0f}}, // 18
			SkyBoxVertex{Vector3{ 1.0f, -1.0f, -1.0f}, Vector2{1.0f, 1.0f}}, // 19
			
			//벡터가 Z+방향										 
			SkyBoxVertex{Vector3{ 1.0f, -1.0f,  1.0f}, Vector2{0.0f, 1.0f}}, // 20
			SkyBoxVertex{Vector3{ 1.0f,  1.0f,  1.0f}, Vector2{0.0f, 0.0f}}, // 21
			SkyBoxVertex{Vector3{-1.0f,  1.0f,  1.0f}, Vector2{1.0f, 0.0f}}, // 22
			SkyBoxVertex{Vector3{-1.0f, -1.0f,  1.0f}, Vector2{1.0f, 1.0f}}, // 23
	};

	//// 정점 버퍼 설정
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	m_VertexCount = ARRAYSIZE(vertices);
	vertexBufferDesc.ByteWidth = sizeof(SkyBoxVertex) * m_VertexCount;
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
	m_VertexBufferStride = sizeof(SkyBoxVertex);
	m_VertexBufferOffset = 0;

	// Render에서 파이프라인에 바인딩할 버텍스 셰이더 생성
	ComPtr<ID3DBlob> vertexShaderBufffer = nullptr; // 버텍스 셰이더 HLSL의 컴파일된 결과를 담을 수 있는 버퍼 객체
	// 컴파일할 셰이더 파일의 이름과 함수, 버전 선택
	HR_T(CompileShaderFromFile(L"VertexShader.hlsl", "main", "vs_4_0", vertexShaderBufffer.GetAddressOf()));
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBufffer->GetBufferPointer(), // 
		vertexShaderBufffer->GetBufferSize(), NULL, m_pVertexShader.GetAddressOf()));

	// 스카이박스가 사용할 버텍스 세이더 생성 및 스카이박스용 버텍스 셰이더 버퍼와 바인딩
	ComPtr<ID3DBlob> skyboxVertexShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"SkyboxVertexShader.hlsl", "main", "vs_4_0", skyboxVertexShaderBuffer.GetAddressOf()));
	HR_T(m_pDevice->CreateVertexShader(skyboxVertexShaderBuffer->GetBufferPointer(),
		skyboxVertexShaderBuffer->GetBufferSize(), NULL, m_pSkyBoxVertexShader.GetAddressOf()));

	//InputLayout 생성
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HR_T(m_pDevice->CreateInputLayout(
		layout,
		ARRAYSIZE(layout),
		vertexShaderBufffer->GetBufferPointer(),
		vertexShaderBufffer->GetBufferSize(),
		m_pInputLayout.GetAddressOf()));

	//스카이 박스용 InputLayout 생성
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

	//HR_T(m_pDevice->CreateBuffer(&cbDesc, nullptr, m_pLightBuffer.GetAddressOf()));
	//m_pDeviceContext->VSSetConstantBuffers(1, 1, m_pLightBuffer.GetAddressOf());
	//m_pDeviceContext->PSSetConstantBuffers(1, 1, m_pLightBuffer.GetAddressOf());

	ComPtr<ID3D11Resource> defualttexture = nullptr;
	//HR_T(DirectX::CreateWICTextureFromFile())

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

	// World Matrix
	m_ScaleTree = Vector3{ 100,100,100 };
	m_TranslationTree = Vector3{ 0,0,100 };
	m_Shininess = 1000;

	m_Camera.m_MoveSpeed = 500;

	//fov 초기화
	fovWidht = m_Width;
	fovHeight = m_Height;
	FieldOfView = 90;
	m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(FieldOfView), (float)m_Width / m_Height, m_near, m_far);

	//fbx 파일 로드
	m_TreeModel.LoadModel(m_handleWindow, m_pDevice.Get(), m_pDeviceContext.Get(), "../Resources/Tree.fbx");
	m_ZeldaModel.LoadModel(m_handleWindow, m_pDevice.Get(), m_pDeviceContext.Get(), "../Resources/zeldaPosed001.fbx");
	m_Character.LoadModel(m_handleWindow, m_pDevice.Get(), m_pDeviceContext.Get(), "../Resources/Character.fbx");
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
