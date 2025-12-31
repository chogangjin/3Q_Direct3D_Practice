#include "framework.h"
#include <array>

#include "DemoGameApp.h"
#include "SkyBoxVertex.h"
#include "../DirectX_3D_Lilbrary/Helper.h"
#include <d3dcompiler.h>
#include <DirectXTK/DDSTextureLoader.h>
#include <directxtk/WICTextureLoader.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib") 
#pragma comment(lib, "d3dcompiler.lib")

//using namespace DirectX;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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

	m_SkinningModel.Update(TimeSystem::GetInstance()->deltaTime);
	m_Robot.Update(TimeSystem::GetInstance()->deltaTime);
	m_PBRModel.Update();
	m_Sphere.Update();
	m_Plane.Update();

	m_Camera.GetViewMatrix(m_ViewMatrix);
	m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(FieldOfView),
		(float)m_Width / m_Height,
		m_near,
		m_far);

	// 그림자 Depth Only Pass Camera 설정
	if (m_bDebugShadow)
	{
		m_ShadowProjection = XMMatrixPerspectiveFovLH(
			XMConvertToRadians(m_ShadowFow),
			m_ShadowViewport.Width / (FLOAT)m_ShadowViewport.Height,
			m_ShadowProjectionNearFar.x, 
			m_ShadowProjectionNearFar.y);
	}
	m_ShadowLookAt = m_Camera.GetCameraPosition() + m_Camera.GetForward() * m_ShadowForwardDistanceFromCamera;
	m_ShadowPos = m_ShadowLookAt + (-m_DirectionalLight * m_ShadowUpDistanceFromLookAt);
	m_ShadowView = XMMatrixLookAtLH(m_ShadowPos, m_ShadowLookAt, Vector3{ 0.0f, 1.0f, 0.0f });
	m_IBL.SetIBLMaterial();
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
	cbuffer.Roughness = m_Roughness;
	cbuffer.Metalness = m_Metalness;
	cbuffer.OverrideMaterial = m_OverrideMaterial;
	//TODO : EXPOSURE 값하고 MAXHDRNITS 값 넣어주어야 함
	cbuffer.Exposure = m_Exposure;
	cbuffer.MaxHDRRNits = m_MonitorMaxNits;


	TransformViewProjection shadowcbuffer = {};
	shadowcbuffer.ShadowView = XMMatrixTranspose(m_ShadowView);
	shadowcbuffer.ShadowProjection = XMMatrixTranspose(m_ShadowProjection);

	//--------------- 디버그용
	// 렌더링 파이프라인 디버그용 nullstate
	ComPtr<ID3D11DepthStencilState> m_pDepthOffState = nullptr;
	D3D11_DEPTH_STENCIL_DESC dsdesc = {};
	dsdesc.DepthEnable = false;
	dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsdesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	dsdesc.StencilEnable = false;
	HR_T(m_pDevice->CreateDepthStencilState(&dsdesc, m_pDepthOffState.GetAddressOf()));

	ComPtr<ID3D11RasterizerState> m_pcullnonestate;
	D3D11_RASTERIZER_DESC rsDesc = {  };
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;
	HR_T(m_pDevice->CreateRasterizerState(&rsDesc, m_pcullnonestate.GetAddressOf()));
	//--------------- 다 쓰면 삭제할것

	cbuffer.World = XMMatrixTranspose(m_WorldMatrix);

	m_pDeviceContext->OMSetRenderTargets(0, NULL, m_pShadowMapDepthStencilView.Get());
	m_pDeviceContext->ClearDepthStencilView(m_pShadowMapDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// 정점 버퍼를 정해둔 버퍼세팅대로 정해둠
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정점을 이어서 그리는 방식
	m_pDeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &m_VertexBufferStride, &m_VertexBufferOffset);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout.Get());							// Input Layout설정
	m_pDeviceContext->VSSetShader(m_pShadowVertexBuffer.Get(), nullptr, 0);				// VertexShader 설정
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());		// ConstantBuffer 초기화
	m_pDeviceContext->VSSetConstantBuffers(2, 1, m_pShadowBuffer.GetAddressOf());		// ConstantBuffer 초기화
	m_pDeviceContext->PSSetShader(nullptr, nullptr, 0);									// PixelShader 설정
	m_pDeviceContext->PSSetSamplers(0, 1, m_pSamplerStateLinear.GetAddressOf());
	m_pDeviceContext->PSSetSamplers(1, 1, m_pSamplerStateClamp.GetAddressOf());
	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
	m_pDeviceContext->UpdateSubresource(m_pShadowBuffer.Get(), 0, nullptr, &shadowcbuffer, 0, 0);

	m_pDeviceContext->RSSetViewports(1, &m_ShadowViewport);
	m_pDeviceContext->OMSetBlendState(m_pAlphaBlendState.Get(), nullptr, 0xffffffff);

	m_SkinningModel.DrawAnimation(&cbuffer, m_pConstantBuffer.Get(), m_pBonePoseBuffer.Get(), m_pBoneOffsetBuffer.Get());
	m_Robot.DrawAnimation(&cbuffer, m_pConstantBuffer.Get(), m_pBonePoseBuffer.Get(), m_pBoneOffsetBuffer.Get());
	m_PBRModel.Draw(&cbuffer, m_pConstantBuffer.Get());
	m_Sphere.Draw(&cbuffer, m_pConstantBuffer.Get());
	m_Plane.Draw(&cbuffer, m_pConstantBuffer.Get());

	// 화면 해당 색으로 칠하기
	// 렌더타겟을 최종 출력 파이프라인에 바인딩
	//m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), color);
	//m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0); // DepthStencilview 초기화, Depth버퍼 1.0f로 초기화
	//m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

	m_pDeviceContext->ClearRenderTargetView(m_pHDRRenderTargetView.Get(), color);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0); // DepthStencilview 초기화, Depth버퍼 1.0f로 초기화
	m_pDeviceContext->OMSetRenderTargets(1, m_pHDRRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());
	m_pDeviceContext->RSSetViewports(1, &m_Viewport);

	//SkyBox
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

	//SkyBox가 카메라 행렬을 따라감
	cbuffer.World = XMMatrixTranspose(DirectX::SimpleMath::Matrix::CreateTranslation(m_Camera.GetCameraPosition()));
 	
	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cbuffer, 0, 0);
	m_pDeviceContext->DrawIndexed(m_Indices, 0, 0);
	m_pDeviceContext->OMSetDepthStencilState(nullptr, 0);
	//m_pDeviceContext->RSSetState(nullptr);
	m_pDeviceContext->RSSetState(m_pcullnonestate.Get());

	// FBX
	cbuffer.World = XMMatrixTranspose(m_WorldMatrix);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout.Get());							// Input Layout설정
	m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);					// VertexShader 설정
	m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);					// PixelShader 설정
	m_pDeviceContext->PSSetShaderResources(7, 1, m_pShadowMapShaderResourceView.GetAddressOf());
	m_pDeviceContext->OMSetBlendState(m_pAlphaBlendState.Get(), nullptr , 0xffffffff);

	m_IBL.SetIBLSRV();
	
	cbuffer.HasNormalMap = m_SkinningModel.m_HasNormalmap;
	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
	m_SkinningModel.DrawAnimation(&cbuffer, m_pConstantBuffer.Get(), m_pBonePoseBuffer.Get(), m_pBoneOffsetBuffer.Get());
	
	//cbuffer.HasNormalMap = m_PBRModel.m_HasNormalMap;
	//m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
	m_Robot.DrawAnimation(&cbuffer, m_pConstantBuffer.Get(), m_pBonePoseBuffer.Get(), m_pBoneOffsetBuffer.Get());
	
	cbuffer.HasNormalMap = m_PBRModel.m_HasNormalMap;
	m_pDeviceContext->PSSetConstantBuffers(0,1,m_pConstantBuffer.GetAddressOf());
	m_PBRModel.Draw(&cbuffer, m_pConstantBuffer.Get());
	
	cbuffer.HasNormalMap = m_PBRModel.m_HasNormalMap;
	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
	m_Sphere.Draw(&cbuffer, m_pConstantBuffer.Get());
	
	cbuffer.HasNormalMap = m_PBRModel.m_HasNormalMap;
	m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
	m_Plane.Draw(&cbuffer, m_pConstantBuffer.Get());

	ID3D11RenderTargetView* nullRTV = nullptr;
	m_pDeviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

	ID3D11ShaderResourceView* nullView = nullptr;
	m_pDeviceContext->PSSetShaderResources(7, 1, &nullView);
	
	// HDR Quad Render
	m_pDeviceContext->RSSetViewports(1, &m_Viewport);
	m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);
	m_pDeviceContext->RSSetViewports(1, &m_Viewport);
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthOffState.Get(), 0);
	m_pDeviceContext->RSSetState(m_pcullnonestate.Get());
	m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), color);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//m_pDeviceContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());
	m_HDRQuad.DrawHDRQuad(m_pDeviceContext.Get(), m_Format, m_pHDRShaderResourceView.Get(),m_pSamplerStateLinear.Get());


	//ImGUI 사용
	ImGuiBeginDraw();
	ImGuiRender();
	ImGuiEndDraw();

	m_pSwapChain->Present(0, 0); // 실제 모니터로 출력
}

bool DemoGameApp::InitD3D()
{
	HRESULT hr = 0;

	DXGI_FORMAT result;
	m_IsHDRSupported = CheckHDRSupporAndMaxNits(m_MonitorMaxNits, result);

	if (!m_forceLDR && m_IsHDRSupported)
	{
		// TODO : HDR로 스왑체인 / 백버퍼 만듦
		m_Format = DXGI_FORMAT_R10G10B10A2_UNORM;
		//DXGI_FORMAT_R16G16B16A16_FLOAT
	}
	else
	{
		// TODO : LDR로 스왑체인 / 백버퍼 만듦
		m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}

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
	
	if (!(m_Format == DXGI_FORMAT_R8G8B8A8_UNORM || m_Format == DXGI_FORMAT_R10G10B10A2_UNORM))
	{
		m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	//SwapChain 생성
	//IDXGIFACTORY7을 사용해서 SWAPCHAIN만드려면 DXGI_SWAP_CHAIN_DESC1 구조체가 필요
	DXGI_SWAP_CHAIN_DESC1 swapdesc = {};
	swapdesc.BufferCount = 2;
	swapdesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//swapdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapdesc.Format = m_Format; // HDR 설정에 따른 포맷 설정
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

	ComPtr<IDXGISwapChain3> pSwapChain3;
	HR_T(m_pSwapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)pSwapChain3.GetAddressOf()));
	if (m_Format == DXGI_FORMAT_R10G10B10A2_UNORM)
	{
		// EOTF = PQ(ST.2084/G2084), 색역(Primaries) = Rec.2020, RGB Full Range
		// 이 스왑체인의 0.0~0.1 값은 선형 RGB나 감마 값이 아니라  PQ로 인코딩 된 HEDR10 신호로 해석하라라는 뜻
		HR_T(pSwapChain3->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020));
	}
	
	//RenderTargetView 생성
	ComPtr<ID3D11Texture2D> BackBuffer;
	HR_T(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)BackBuffer.GetAddressOf()));
	m_pDevice->CreateRenderTargetView(BackBuffer.Get(), nullptr, m_pRenderTargetView.GetAddressOf());

	// 뷰포트 설정
	//D3D11_VIEWPORT viewport = {};
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width = (float)m_Width;
	m_Viewport.Height = (float)m_Height;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &m_Viewport);

	// ShadowMap Viewport 생성
	m_ShadowViewport.TopLeftX = 0;
	m_ShadowViewport.TopLeftY = 0;
	m_ShadowViewport.Width = 8192;
	m_ShadowViewport.Height = 8192;
	m_ShadowViewport.MinDepth = 0.0f;
	m_ShadowViewport.MaxDepth = 1.0f;
	 
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
	D3D11_DEPTH_STENCIL_DESC skyboxDepthStencilDesc = {};
	skyboxDepthStencilDesc.DepthEnable = true;
	skyboxDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	skyboxDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // 깊이데이터를 기존 깊이 데이터와 비교, 원본 데이터가 대상데이터보다 작거나 같으면 비교 통과
	m_pDevice->CreateDepthStencilState(&skyboxDepthStencilDesc, m_pSkyBoxDepthStencilState.GetAddressOf());

	//Shadow map 생성
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = (UINT)m_ShadowViewport.Width; // 텍스쳐 해상도
	textureDesc.Height = (UINT)m_ShadowViewport.Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS; // Depth + Shader Resource호환
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	//                      깊이값 기록 용도          | 셰이더에서 텍스쳐 슬롯에 설정할 용도
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; 
	HR_T(m_pDevice->CreateTexture2D(&textureDesc, nullptr, m_pShadowMap.GetAddressOf()));

	// 그림자 전용 depth/stencil state 생성
	depthStencilViewDesc = {};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	HR_T(m_pDevice->CreateDepthStencilView(m_pShadowMap.Get(), &depthStencilViewDesc, m_pShadowMapDepthStencilView.GetAddressOf()));

	// 그림자 ShaderResourceView 생성
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	HR_T(m_pDevice->CreateShaderResourceView(m_pShadowMap.Get(), &shaderResourceViewDesc, m_pShadowMapShaderResourceView.GetAddressOf())); // 깊이값 기록을 설정하기 위한 객체

	// HDR 전용 렌더타겟, DSV 생성, 텍스쳐 생성
	D3D11_TEXTURE2D_DESC HDRdesc = {};
	HDRdesc.Width = static_cast<UINT>(m_Width);
	HDRdesc.Height = static_cast<UINT>(m_Height);
	HDRdesc.MipLevels = 1;
	HDRdesc.ArraySize = 1;
	HDRdesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	HDRdesc.SampleDesc.Count = 1; // MSAA 없음
	HDRdesc.SampleDesc.Quality = 0;
	HDRdesc.Usage = D3D11_USAGE_DEFAULT;
	HDRdesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HR_T(m_pDevice->CreateTexture2D(&HDRdesc, nullptr, m_pHDRRenderTarget.GetAddressOf()));
	HR_T(m_pDevice->CreateRenderTargetView(m_pHDRRenderTarget.Get(), nullptr, m_pHDRRenderTargetView.GetAddressOf()));
	HR_T(m_pDevice->CreateShaderResourceView(m_pHDRRenderTarget.Get(), nullptr, m_pHDRShaderResourceView.GetAddressOf()));

	// Rasterizer State
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthClipEnable = true;
	m_pDevice->CreateRasterizerState(&rasterizerDesc, m_pRasterizerState.GetAddressOf()); // 셰이더에서 깊이 버퍼를 슬롯에 설정하고 사용하기 위한 객체

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = true;

	D3D11_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = true;
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

	m_HDRQuad.SetDevice(m_pDevice.Get());
	m_HDRQuad.CreateQuadVertexShader();
	m_HDRQuad.CreateHDRandLDRPixelShader();

	return true;
}

void DemoGameApp::UnInitD3D()
{
	
}

bool DemoGameApp::CheckHDRSupporAndMaxNits(float& outmaxNits, DXGI_FORMAT& outFormat)
{
	ComPtr<IDXGIFactory4> pFactory;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
	if (FAILED(hr))
	{
		LOG_ERRORA("Error : DXGI Factory Create Failed!\n");
		return false;
	}

	//2. 주 그래픽 어댑터 (0번) 열
	ComPtr<IDXGIAdapter1> pAdapter;
	UINT adapterIndex = 0;
	while (pFactory->EnumAdapters1(adapterIndex, &pAdapter)!= DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		pAdapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			adapterIndex++;
			pAdapter.Reset();
			continue;
		}
		break;
	}

	if (!pAdapter)
	{
		LOG_ERRORA("Error: 유효한 하드웨어 어댑터를 찾을 수 없습니다. \n");
		return false;
	}

	//3. 주 모니터 출력(0번)열거
	ComPtr<IDXGIOutput> pOutput;
	hr = pAdapter->EnumOutputs(0, &pOutput);
	if (FAILED(hr))
	{
		LOG_ERRORA("Error: 주 모니터 출력(Output 0)을 찾을 수 없습니다 \n");
		return false;
	}

	// 4. HDR 정보를 얻기 위해 IDXGIOutput6로 쿼리
	ComPtr<IDXGIOutput6> pOutput6;
	hr = pOutput.As(&pOutput6);
	if (FAILED(hr))
	{
		printf("INFO: IDXGIOutput6 인터페이스를 얻을 수 없습니다. HDR 정보를 얻을 수 없습니다.\n");
		outmaxNits = 100.0f;
		outFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		return false;
	}

	// 5. DXGI_OUTPUT_DESC1에서 HDR 정보 확인
	DXGI_OUTPUT_DESC1 desc1 = {};
	hr = pOutput6->GetDesc1(&desc1);
	if (FAILED(hr))
	{
		printf("Error : GetDesc1 호출 실패");
		return false;
	}

	// 6. HDR 활성화 조건 분석
	bool isHDRColorSpace = (desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
	outmaxNits = (float)desc1.MaxLuminance;

	// OS가 HDR을 켰을 때 MaxLuminantce는 100Nits(SDR 기준)을 초과
	bool isHDRActive = outmaxNits > 100.0f;

	if (isHDRColorSpace && isHDRActive)
	{
		// 최종 판단 : HDR 지원 및 OS 활성화
		outFormat = DXGI_FORMAT_R10G10B10A2_UNORM; // HDR format
		printf("Success : HDR 활성화 됨. MaxNits : %.1f, Format : DXGI_FORMAT_R10G10B10A2_UNORM\n", outmaxNits);
		return true;
	}
	else
	{
		//HDR 지원 안함 또는 OS에서 비활성화
		outmaxNits = 100.0f; // SDR 기본값
		outFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // SDR 포맷 설정
		printf("Info : HDR 비활성화. MaxNIts : 100.0f, Format : DXGI_FORMAT_R8G8B8A8_UNORM\n");
		return false;
	}
	return true;
}

bool DemoGameApp::InitScene()
{
	// Render에서 파이프라인에 바인딩할 버텍스 셰이더 생성
	ComPtr<ID3DBlob> vertexShaderBufffer = nullptr; // 버텍스 셰이더 HLSL의 컴파일된 결과를 담을 수 있는 버퍼 객체
	
	// 컴파일할 셰이더 파일의 이름과 함수, 버전 선택
	HR_T(CompileShaderFromFile(L"VertexShader.hlsl", "main", "vs_5_0", vertexShaderBufffer.GetAddressOf()));
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBufffer->GetBufferPointer(), // 
		vertexShaderBufffer->GetBufferSize(), NULL, m_pVertexShader.GetAddressOf()));

	//InputLayout 생성
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BINORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT,  0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HR_T(m_pDevice->CreateInputLayout(
		layout,
		ARRAYSIZE(layout),
		vertexShaderBufffer->GetBufferPointer(),
		vertexShaderBufffer->GetBufferSize(),
		m_pInputLayout.GetAddressOf()));

	vertexShaderBufffer = nullptr;
	HR_T(CompileShaderFromFile(L"ShadowVertexShader.hlsl", "main", "vs_5_0", vertexShaderBufffer.GetAddressOf()));
	HR_T(m_pDevice->CreateVertexShader(vertexShaderBufffer->GetBufferPointer(),
		vertexShaderBufffer->GetBufferSize(), NULL, m_pShadowVertexBuffer.GetAddressOf()));

	// 픽셀 셰이더 생성
	ComPtr<ID3DBlob> pixelShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"PixelShader.hlsl", "main", "ps_5_0", pixelShaderBuffer.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, m_pPixelShader.GetAddressOf()));

	// 상수 버퍼 생성
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT; // 기본으로 설정
	cbDesc.ByteWidth = sizeof(ConstantBuffer); // 사이즈 설정
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // 상수버퍼로 바인딩
	cbDesc.CPUAccessFlags = 0; // cpu에 대한 엑세스가 필요한지 여부 -> 현재는 필요없으니 0으로 설정
	HR_T(m_pDevice->CreateBuffer(&cbDesc, nullptr, m_pConstantBuffer.GetAddressOf())); // 버퍼 생성
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf()); // 상수버퍼를 Vertex Shader에 연결

	//BoneOffset을 담는 버퍼 생성
	cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(BoneMatrixContainer);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&cbDesc, nullptr, m_pBoneOffsetBuffer.GetAddressOf()));
	m_pDeviceContext->VSSetConstantBuffers(3, 1, m_pBoneOffsetBuffer.GetAddressOf());

	//Bone Pose Matrix를 담는 버퍼 생성
	cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(BoneMatrixContainer);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	//cbDesc.StructureByteStride = sizeof(Matrix);
	HR_T(m_pDevice->CreateBuffer(&cbDesc, nullptr, m_pBonePoseBuffer.GetAddressOf()));
	//m_pDeviceContext->UpdateSubresource(m_pBoneMatrixBuffer.Get(), 0, nullptr, &m_ZeldaModel.m_SkeletonPose.modelMatrix[0], 0, 0);
	m_pDeviceContext->VSSetConstantBuffers(4, 1, m_pBonePoseBuffer.GetAddressOf());

	// 그림자 맵 전용 상수 버퍼 생성
	cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(TransformViewProjection);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	HR_T(m_pDevice->CreateBuffer(&cbDesc, nullptr, m_pShadowBuffer.GetAddressOf()));
	m_pDeviceContext->VSSetConstantBuffers(2, 1, m_pShadowBuffer.GetAddressOf());

	ComPtr<ID3D11Resource> skycubeTexture = nullptr;
	HR_T(DirectX::CreateDDSTextureFromFile(m_pDevice.Get(), L"../Resources/IBL/OutputEnvHDR.dds", skycubeTexture.GetAddressOf(), m_pSkyBoxShaderResourceView.GetAddressOf()));

	// SamplerState 생성
	D3D11_SAMPLER_DESC samplerStateDesc = {};
	samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerStateDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerStateDesc.MinLOD = 0;
	samplerStateDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR_T(m_pDevice->CreateSamplerState(&samplerStateDesc, m_pSamplerStateLinear.GetAddressOf()));

	//ClampSampler 생성
	samplerStateDesc = {};
	samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerStateDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerStateDesc.MinLOD = 0.0f;
	samplerStateDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR_T(m_pDevice->CreateSamplerState(&samplerStateDesc, m_pSamplerStateClamp.GetAddressOf()));


	//light 받는 면적 조정
	m_Shininess = 2;

	//카메라 이동 속도
	m_Camera.m_MoveSpeed = 500;

	//fov 초기화
	fovWidht =  (float)m_Width;
	fovHeight = (float)m_Height;
	FieldOfView = 90;
	m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(FieldOfView), (float)m_Width / m_Height, m_near, m_far);

	//fbx 파일 로드
	m_SkinningModel.LoadModel(m_handleWindow, m_pDevice.Get(), m_pDeviceContext.Get(), "../Resources/SkinningTest.fbx");
	m_Robot.LoadModel(m_handleWindow, m_pDevice.Get(), m_pDeviceContext.Get(), "../Resources/BoxHuman.fbx");
	m_PBRModel.LoadModel(m_handleWindow, m_pDevice.Get(), m_pDeviceContext.Get(), "../Resources/char.fbx");
	m_Sphere.LoadModel(m_handleWindow, m_pDevice.Get(), m_pDeviceContext.Get(), "../Resources/sphere.fbx");
	m_Plane.LoadModel(m_handleWindow, m_pDevice.Get(), m_pDeviceContext.Get(), "../Resources/Plane.fbx");

	m_Plane.m_Scale *= 10;

	m_Sphere.m_Translation = { 100, 50, 0 };
	m_PBRModel.m_Translation = { 200, 30, 0 };

	m_IBL.SetDeviceandDeviceContext(m_pDevice.Get(), m_pDeviceContext.Get());
	m_IBL.LoadIBLMaterialFromFilePath(L"Output");

	//스카이큐브 설정
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
		//SkyBox
		//Normal Y+
		//                   position                texture         
		SkyBoxVertex{Vector3{-1.0f,  1.0f, -1.0f}, Vector2{0.0f, 1.0f}}, // 0
		SkyBoxVertex{Vector3{-1.0f,  1.0f,  1.0f}, Vector2{0.0f, 0.0f}}, // 1
		SkyBoxVertex{Vector3{ 1.0f,  1.0f,  1.0f}, Vector2{1.0f, 0.0f}}, // 2
		SkyBoxVertex{Vector3{ 1.0f,  1.0f, -1.0f}, Vector2{1.0f, 1.0f}}, // 3
		
		//Normal Y-
		SkyBoxVertex{Vector3{-1.0f, -1.0f,  1.0f}, Vector2{0.0f, 1.0f}}, // 4
		SkyBoxVertex{Vector3{-1.0f, -1.0f, -1.0f}, Vector2{0.0f, 0.0f}}, // 5
		SkyBoxVertex{Vector3{ 1.0f, -1.0f, -1.0f}, Vector2{1.0f, 0.0f}}, // 6
		SkyBoxVertex{Vector3{ 1.0f, -1.0f,  1.0f}, Vector2{1.0f, 1.0f}}, // 7
		
		//Normal X-
		SkyBoxVertex{Vector3{-1.0f, -1.0f,  1.0f}, Vector2{0.0f, 1.0f}}, // 8
		SkyBoxVertex{Vector3{-1.0f,  1.0f,  1.0f}, Vector2{0.0f, 0.0f}}, // 9
		SkyBoxVertex{Vector3{-1.0f,  1.0f, -1.0f}, Vector2{1.0f, 0.0f}}, // 10
		SkyBoxVertex{Vector3{-1.0f, -1.0f, -1.0f}, Vector2{1.0f, 1.0f}}, // 11
		
		//Normal X+
		SkyBoxVertex{Vector3{ 1.0f, -1.0f, -1.0f}, Vector2{0.0f, 1.0f}}, // 12
		SkyBoxVertex{Vector3{ 1.0f,  1.0f, -1.0f}, Vector2{0.0f, 0.0f}}, // 13
		SkyBoxVertex{Vector3{ 1.0f,  1.0f,  1.0f}, Vector2{1.0f, 0.0f}}, // 14
		SkyBoxVertex{Vector3{ 1.0f, -1.0f,  1.0f}, Vector2{1.0f, 1.0f}}, // 15
		
		//Normal Z-
		SkyBoxVertex{Vector3{-1.0f, -1.0f, -1.0f}, Vector2{0.0f, 1.0f}}, // 16
		SkyBoxVertex{Vector3{-1.0f,  1.0f, -1.0f}, Vector2{0.0f, 0.0f}}, // 17
		SkyBoxVertex{Vector3{ 1.0f,  1.0f, -1.0f}, Vector2{1.0f, 0.0f}}, // 18
		SkyBoxVertex{Vector3{ 1.0f, -1.0f, -1.0f}, Vector2{1.0f, 1.0f}}, // 19
		
		//Normal Z+
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

	// 스카이박스가 사용할 버텍스 세이더 생성 및 스카이박스용 버텍스 셰이더 버퍼와 바인딩
	ComPtr<ID3DBlob> skyboxVertexShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"SkyboxVertexShader.hlsl", "main", "vs_5_0", skyboxVertexShaderBuffer.GetAddressOf()));
	HR_T(m_pDevice->CreateVertexShader(skyboxVertexShaderBuffer->GetBufferPointer(),
		skyboxVertexShaderBuffer->GetBufferSize(), NULL, m_pSkyBoxVertexShader.GetAddressOf()));


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

	//스카이큐브를 그릴 점들의 인덱스 설정
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

	// 스카이박스 전용 픽셀셰이더 생성
	ComPtr<ID3DBlob> skyboxPixelShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"SkyBoxPixelShader.hlsl", "main", "ps_5_0", skyboxPixelShaderBuffer.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(
		skyboxPixelShaderBuffer->GetBufferPointer(),
		skyboxPixelShaderBuffer->GetBufferSize(), NULL, m_pSkyBoxPixelShader.GetAddressOf()));
}

//Imgui 설정
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
	ImGui::Begin("IBL");
	ImGui::SeparatorText("Skinning");
	ImGui::DragFloat3("Mixamo Position", &m_SkinningModel.m_Translation.x, 1.0f, -10000.0f, 10000.0f);
	ImGui::DragFloat3("Mixamo Rotation", &m_SkinningModel.m_Rotation.x, 0.01f, -360.0f, 360.0f);
	ImGui::DragFloat3("Mixamo Scale", &m_SkinningModel.m_Scale.x, 0.1f, 0.1f, 100.0f);
	
	ImGui::SeparatorText("BoxHuman");
	ImGui::DragFloat3("Robot Position", &m_Robot.m_Translation.x, 1.0f, -10000.0f, 10000.0f);
	ImGui::DragFloat3("Robot Rotation", &m_Robot.m_Rotation.x, 0.01f, -360.0f, 360.0f);
	ImGui::DragFloat3("Robot Scale", &m_Robot.m_Scale.x, 0.1f, 0.1f, 100.0f);

	ImGui::SeparatorText("PBR Model");
	ImGui::DragFloat3("PBR Position", &m_PBRModel.m_Translation.x, 1.0f, -10000.0f, 10000.0f);
	ImGui::DragFloat3("PBR Rotation", &m_PBRModel.m_Rotation.x, 0.01f, -360.0f, 360.0f);
	ImGui::DragFloat3("PBR Scale", &m_PBRModel.m_Scale.x, 0.1f, 0.1f, 100.0f);

	ImGui::SeparatorText("Sphere");
	ImGui::DragFloat3("Sphere Position", &m_Sphere.m_Translation.x, 1.0f, -10000.0f, 10000.0f);
	ImGui::DragFloat3("Sphere Rotation", &m_Sphere.m_Rotation.x, 0.01f, -360.0f, 360.0f);
	ImGui::DragFloat3("Sphere Scale", &m_Sphere.m_Scale.x, 0.1f, 0.1f, 100.0f);

	ImGui::NewLine();
	ImGui::SeparatorText("Light");
	ImGui::DragFloat3("Directional Light", &m_DirectionalLight.x, 0.01f, -1.0f, 1.0f);
	ImGui::ColorEdit4("Directional Light Color", &m_LightColor.x);
	ImGui::Checkbox("ChangeIBL", &m_IBL.m_IsChangeIBL);
	ImGui::DragFloat("LightPower", &m_Shininess, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Exposure", &m_Exposure, 0.01f, -5.0f, 5.0f);

	ImGui::NewLine();
	ImGui::SeparatorText("Camera Setting");
	ImGui::DragFloat("Near", &m_near, 0.1f, 0.1f, m_far - 0.2f);
	ImGui::DragFloat("Far", &m_far, 0.1f, m_near + 0.2f, 1000.0f);
	ImGui::DragFloat("FoV", &FieldOfView, 0.1f, 0.1f, 360.0f);

	ImGui::SeparatorText("PBR Setting");
	ImGui::DragFloat("Roughness", &m_Roughness, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Metalness", &m_Metalness, 0.01f, 0.0f, 1.0f);
	ImGui::Checkbox("Override Material", &m_OverrideMaterial);
	ImGui::Checkbox("Change IBL", &m_IBL.m_IsChangeIBL);
	ImGui::End();

	//For Debug Shadow
	ImGui::Begin("DebugShadow");
	ImGui::Text("Distance");
	ImGui::DragFloat("Distance from Camera", &m_ShadowForwardDistanceFromCamera, 0.1f, 0.1f, 1000.0f);
	ImGui::DragFloat("Distance from LookAt", &m_ShadowUpDistanceFromLookAt, 0.1f, 0.1f, 1000.0f);
	ImGui::SeparatorText("Shadow Near Far");
	ImGui::DragFloat("Shadow Near", &m_ShadowProjectionNearFar.x, 0.1f, 0.1f, m_ShadowProjectionNearFar.y- 0.2f);
	ImGui::DragFloat("Shadow Far", &m_ShadowProjectionNearFar.y, 0.1f, m_ShadowProjectionNearFar.x + 0.2f, 1000.0f);
	ImGui::DragFloat("Shadow Fov", &m_ShadowFow, 0.1f);
	ImGui::Image((ImTextureID)(intptr_t)m_pShadowMapShaderResourceView.Get(), ImVec2(300, 300));
	ImGui::End();
}

void DemoGameApp::ImGuiEndDraw()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
