#include "framework.h"
#include "GBuffer.h"
#include "../DirectX_3D_Lilbrary/Helper.h"

bool GBuffer::CreateGBuffer(float width,float height)
{
	struct RTDesc
	{
		DXGI_FORMAT format;
	};

	RTDesc formats[m_Count] =
	{
		{ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB },
		{ DXGI_FORMAT_R8G8B8A8_UNORM },
		{ DXGI_FORMAT_R16G16B16A16_FLOAT }
	};

	for (int i = 0; i < m_Count; i++)
	{
		D3D11_TEXTURE2D_DESC td = {};
		td.Width = width;
		td.Height = height;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = formats[i].format;
		td.SampleDesc.Count = 1;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		HR_T(m_pDevice->CreateTexture2D(&td, nullptr, m_pGTextures[i].GetAddressOf()));
		HR_T(m_pDevice->CreateRenderTargetView(m_pGTextures[i].Get(), nullptr, m_pRenderTargetView[i].GetAddressOf()));
		HR_T(m_pDevice->CreateShaderResourceView(m_pGTextures[i].Get(), nullptr, m_pShaderResourceView[i].GetAddressOf()));
	}
	return true;
}

bool GBuffer::CreateGBufferShaders()
{
	ComPtr<ID3DBlob> vsBlob;
	HR_T(CompileShaderFromFile(L"GBufferVertexShader.hlsl", "main", "vs_5_0", vsBlob.GetAddressOf()));
	HR_T(m_pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, m_pVertexShader.GetAddressOf()));

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32_SINT,     0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_pInputLayout.GetAddressOf()));

	ComPtr<ID3DBlob> psBlob;
	CompileShaderFromFile(L"GBufferPixelShader.hlsl", "main", "ps_5_0", psBlob.GetAddressOf());
	HR_T(m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));
	
	return true;
}
