#include "framework.h"
#include "HDR_Quad.h"
#include "../DirectX_3D_Lilbrary/Helper.h"

void HDR_Quad::CreateHDRQuad()
{

	
}

void HDR_Quad::CreateQuadVertexShader()
{
	struct QuadVertex
	{
		Vector3 position;
		Vector2 texture;
	};

	QuadVertex quadVertices[] =
	{
		QuadVertex(Vector3(-1.0f,  1.0f, 1.0f), Vector2(0.0f,0.0f)),	// Left Top 
		QuadVertex(Vector3(1.0f,  1.0f, 1.0f), Vector2(1.0f, 0.0f)),	// Right Top
		QuadVertex(Vector3(-1.0f, -1.0f, 1.0f), Vector2(0.0f, 1.0f)),	// Left Bottom
		QuadVertex(Vector3(1.0f, -1.0f, 1.0f), Vector2(1.0f, 1.0f))		// Right Bottom
	};

	D3D11_BUFFER_DESC vbDesc = { };
	vbDesc.ByteWidth = sizeof(QuadVertex) * ARRAYSIZE(quadVertices);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = quadVertices;
	HR_T(m_pDevice->CreateBuffer(&vbDesc, &vbData, m_pQuadVertexBuffer.GetAddressOf()));
	m_QuadVertexBufferStride = sizeof(QuadVertex);
	m_QuadVertexBufferOffset = 0;

	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ComPtr<ID3D10Blob> vertexShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"QuadVertexShader.hlsl", "main", "vs_5_0", vertexShaderBuffer.GetAddressOf()));
	HR_T(m_pDevice->CreateInputLayout(inputLayout, ARRAYSIZE(inputLayout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), m_pQuadInputLayout.GetAddressOf()));

	HR_T(m_pDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, m_pQuadVertexShader.GetAddressOf()));
}

void HDR_Quad::CreateHDRandLDRPixelShader()
{
	WORD quadIndices[] =
	{
		0,1,2,
		2,1,3
	};
	m_NumQuadIndices = ARRAYSIZE(quadIndices);
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.ByteWidth = sizeof(WORD) * ARRAYSIZE(quadIndices);
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = quadIndices;
	HR_T(m_pDevice->CreateBuffer(&ibDesc, &ibData, m_pQuadIndexBuffer.GetAddressOf()));

	ComPtr<ID3D10Blob> pixelShaderBuffer = nullptr;

	HR_T(CompileShaderFromFile(L"HDRPixelShader.hlsl", "main", "ps_5_0", &pixelShaderBuffer));
	HR_T(m_pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, m_pHDRPixelShader.GetAddressOf()));
	//pixelShaderBuffer.ReleaseAndGetAddressOf();
	pixelShaderBuffer.Reset();

	HR_T(CompileShaderFromFile(L"LDRPixelShader.hlsl", "main", "ps_5_0", &pixelShaderBuffer));
	HR_T(m_pDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, m_pLDRPixelShader.GetAddressOf()));
}

void HDR_Quad::DrawHDRQuad(ID3D11DeviceContext* pDeviceContext,DXGI_FORMAT format, ID3D11ShaderResourceView* pShaderResourceView, ID3D11SamplerState* pSamplerState)
{
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDeviceContext->IASetVertexBuffers(0, 1, m_pQuadVertexBuffer.GetAddressOf(), &m_QuadVertexBufferStride, &m_QuadVertexBufferOffset);
	pDeviceContext->IASetInputLayout(m_pQuadInputLayout.Get());
	pDeviceContext->IASetIndexBuffer(m_pQuadIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	pDeviceContext->VSSetShader(m_pQuadVertexShader.Get(), nullptr, 0);

	switch (format)
	{
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		pDeviceContext->PSSetShader(m_pHDRPixelShader.Get(), nullptr, 0);
		break;
	default:
		pDeviceContext->PSSetShader(m_pLDRPixelShader.Get(), nullptr, 0);
		break;
	}
	//pDeviceContext->PSSetShader(m_pHDRPixelShader.Get(), nullptr, 0);
	
	ID3D11ShaderResourceView* srv[] = {pShaderResourceView};
	pDeviceContext->PSSetShaderResources(14, 1, srv);
	
	ID3D11SamplerState* samplers[] = { pSamplerState };
	pDeviceContext->PSSetSamplers(0, 1, samplers);
	
	pDeviceContext->DrawIndexed(m_NumQuadIndices, 0, 0);
	
	ID3D11ShaderResourceView* nullSRV = nullptr;
	pDeviceContext->PSSetShaderResources(14, 1, &nullSRV);
}

void HDR_Quad::DrawHDRQuadDeffered(ID3D11DeviceContext* pDeviceContext, DXGI_FORMAT format, ID3D11ShaderResourceView* pShaderResourceView, ID3D11SamplerState* pSamplerState)
{
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDeviceContext->IASetVertexBuffers(0, 1, m_pQuadVertexBuffer.GetAddressOf(), &m_QuadVertexBufferStride, &m_QuadVertexBufferOffset);
	pDeviceContext->IASetInputLayout(m_pQuadInputLayout.Get());
	pDeviceContext->IASetIndexBuffer(m_pQuadIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	pDeviceContext->VSSetShader(m_pQuadVertexShader.Get(), nullptr, 0);

	switch (format)
	{
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		pDeviceContext->PSSetShader(m_pHDRPixelShader.Get(), nullptr, 0);
		break;
	default:
		pDeviceContext->PSSetShader(m_pLDRPixelShader.Get(), nullptr, 0);
		break;
	}


	ID3D11ShaderResourceView* srv[] = { pShaderResourceView };
	pDeviceContext->PSSetShaderResources(14, 1, srv);

	ID3D11SamplerState* samplers[] = { pSamplerState };
	pDeviceContext->PSSetSamplers(0, 1, samplers);

	pDeviceContext->DrawIndexed(m_NumQuadIndices, 0, 0);

	ID3D11ShaderResourceView* nullSRV = nullptr;
	pDeviceContext->PSSetShaderResources(14, 1, &nullSRV);
}
