#pragma once
#include "framework.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Microsoft::WRL;

struct Vertex
{
	Vector3 position;
	Vector2 texture;

	//// Assimp부터 사용하지 않음
	Vector3 Normal;
	Vector3 Tangent;
	Vector3 BiNormal;

	Vertex(float x, float y, float z) : position(x, y, z) {}
	Vertex(Vector3 position, Vector2 texture = {}, Vector3 normal = {}, Vector3 tangent = {}, Vector3 binormal = {}) : position(position), texture(texture), Normal(normal), Tangent(tangent), BiNormal(binormal) {}
};

struct Texture
{
	std::string type; // 텍스쳐 타입 -> Diffuse, Normal, Specular 등등
	std::string path; // 파일을 불러올 경로
	ComPtr<ID3D11ShaderResourceView> m_pTextureSRV;
};

class Mesh
{
public:
	ID3D11Device* m_pDevice;
	std::vector<Vertex> m_Vertices;
	std::vector<UINT> m_Indices;
	std::vector<Texture> m_Textures;

	Mesh(ID3D11Device* device, const std::vector<Vertex>& vertices, const std::vector<UINT>& indices, const std::vector<Texture>& textures) :
		m_pDevice(device),
		m_Vertices(vertices),
		m_Indices(indices),
		m_Textures(textures)
	{
		SetVertexIndexBuffer(m_pDevice);
	}

	void DrawMesh(ID3D11DeviceContext* deviceContext)
	{
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);	// 버텍스 버퍼 입력 조립

		deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		// 인덱스 버퍼 입력 조립
		for (UINT i = 0; i < m_Textures.size(); i++)
		{
			if (m_Textures[i].type == "texture_diffuse")
			{
				deviceContext->PSSetShaderResources(0, 1, m_Textures[i].m_pTextureSRV.GetAddressOf()); // SRV 벡터의 첫번째 주소를 전달해서 쭉 읽도록
			}
			if (m_Textures[i].type == "texture_specular")
			{
				deviceContext->PSSetShaderResources(3, 1, m_Textures[i].m_pTextureSRV.GetAddressOf());
			}
			if (m_Textures[i].type == "texture_ambient")
			{
				deviceContext->PSSetShaderResources(4, 1, m_Textures[i].m_pTextureSRV.GetAddressOf());
			}
			if (m_Textures[i].type == "texture_emissive")
			{
				deviceContext->PSSetShaderResources(5, 1, m_Textures[i].m_pTextureSRV.GetAddressOf());
			}
			else
			{
				ID3D11ShaderResourceView* nullSRV = nullptr;
				deviceContext->PSSetShaderResources(5, 1, &nullSRV);
			}
			//if (m_Textures[i].type == "texture_opacity") 
			{
				deviceContext->PSSetShaderResources(6, 1, m_Textures[i].m_pTextureSRV.GetAddressOf());
			}
			//else
			//{
			//	deviceContext->PSSetShaderResources(6, 1, m_Textures[i].m_pTextureSRV.GetAddressOf());
			//}
		}
		
		deviceContext->DrawIndexed(static_cast<UINT>(m_Indices.size()), 0, 0);						// 인덱스 버퍼에 저장되어있는 인덱스대로 그리기
	}
private:
	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ComPtr<ID3D11Buffer> m_pIndexBuffer;

	void SetVertexIndexBuffer(ID3D11Device* device)
	{
		HRESULT hr;

		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;			// GPU에서만 읽을 수 있고, cpu에서 엑세스 x, 작성 또는 변경 불가
		vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * m_Vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;  // 버텍스 버퍼로 사용
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags;								// mip-map설정 관련 플래그

		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &m_Vertices[0]; // 주소 넘겨서 버텍스 정보를 벡터 첫번째부터 쭉 읽게 함

		hr = device->CreateBuffer(&vertexBufferDesc, &BufferData, &m_pVertexBuffer); // 버텍스 버퍼 생성
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to Create Vertex Buffer!");
		}

		D3D11_BUFFER_DESC indexBufferDesc = {};
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(UINT) * m_Indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		BufferData = {};
		BufferData.pSysMem = &m_Indices[0];

		hr = device->CreateBuffer(&indexBufferDesc, &BufferData, &m_pIndexBuffer);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to Create Index Buffer!");
		}
	}
};
