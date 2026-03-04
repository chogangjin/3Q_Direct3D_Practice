#include "framework.h"
#include "SkyBoxVertex.h"
#include "../DirectX_3D_Lilbrary/Helper.h"
#include <DirectXTK/DDSTextureLoader.h>

void SkyBox::CreateShaaders()
{
	ComPtr<ID3DBlob> vsblob;
	HR_T(CompileShaderFromFile(L"SkyboxVertexShader.hlsl", "main", "vs_5_0", vsblob.GetAddressOf()));
	HR_T(m_pDevice->CreateVertexShader(vsblob->GetBufferPointer(), vsblob->GetBufferSize(),
		NULL, m_pVertexShader.GetAddressOf()));

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vsblob->GetBufferPointer(),
		vsblob->GetBufferSize(), m_pInputLayout.GetAddressOf()));
	
	ComPtr<ID3DBlob> psblob;
	HR_T(CompileShaderFromFile(L"SkyBoxPixelShader.hlsl", "main", "ps_5_0", psblob.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(psblob->GetBufferPointer(), psblob->GetBufferSize(), NULL,
		m_pPixelShader.GetAddressOf()));

}

void SkyBox::CreateBuffers()
{
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
	HR_T(m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, m_pVertexBuffer.GetAddressOf()));

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
	
	m_VertexBufferStride = sizeof(SkyBoxVertex);
	m_VertexBufferOffset = 0;
}

void SkyBox::CreateStates()
{
	D3D11_DEPTH_STENCIL_DESC skyboxDepthStencilDesc = {};
	skyboxDepthStencilDesc.DepthEnable = true;
	skyboxDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	skyboxDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // 깊이데이터를 기존 깊이 데이터와 비교, 원본 데이터가 대상데이터보다 작거나 같으면 비교 통과
	m_pDevice->CreateDepthStencilState(&skyboxDepthStencilDesc, m_pDepthStencilState.GetAddressOf());

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthClipEnable = true;
	m_pDevice->CreateRasterizerState(&rasterizerDesc, m_pRasterizerState.GetAddressOf()); // 셰이더에서 깊이 버퍼를 슬롯에 설정하고 사용하기 위한 객체
}

void SkyBox::CreateTexture(std::wstring filepath)
{
	ComPtr<ID3D11Resource> texture;
	//HR_T(DirectX::CreateDDSTextureFromFile(m_pDevice, L"../Resources/IBL/OutputEnvHDR.dds", texture.GetAddressOf(), m_pShaderResourceView.GetAddressOf()));
	HR_T(DirectX::CreateDDSTextureFromFile(m_pDevice, filepath.c_str(), texture.GetAddressOf(), m_pShaderResourceView.GetAddressOf()));
}

void SkyBox::DrawSkyBox(ID3D11Buffer* pConstantBuffer)
{
	//m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilState.Get(), 0);
	//m_pDeviceContext->RSSetState(m_pRasterizerState.Get());
	//m_pDeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &m_VertexBufferStride, &m_VertexBufferOffset);
	//m_pDeviceContext->IASetInputLayout(m_pInputLayout.Get());
	//m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);	// Index Buffer에 인덱스들 값 설정
	//m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
	//m_pDeviceContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);		// ConstantBuffer 초기화
	//m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
	//m_pDeviceContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);
	//m_pDeviceContext->PSSetShaderResources(1, 1, m_pShaderResourceView.GetAddressOf());

	//m_pDeviceContext->DrawIndexed(m_Indices, 0, 0);
}
