#pragma once
#include "DeviceBase.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Microsoft::WRL;

struct SkyBoxVertex
{
	Vector3 position;
	Vector2	texture;
	Vector3 Normal;
	Vector3 Tangent;
	Vector3 BiNormal;

	SkyBoxVertex(Vector3 position, Vector2 texture = {}, Vector3 Normal = {}, Vector3 tangent = {}, Vector3 binormal = {}) : position(position), texture(texture), Normal(Normal), Tangent(tangent), BiNormal(binormal) {}
};

class SkyBox : public DeviceBase
{
public:
	ComPtr<ID3D11VertexShader> m_pVertexShader;
	ComPtr<ID3D11PixelShader> m_pPixelShader;
	ComPtr<ID3D11InputLayout> m_pInputLayout;

	ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;
	ComPtr<ID3D11RasterizerState> m_pRasterizerState;
	ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView;

	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ComPtr<ID3D11Buffer> m_pIndexBuffer;
	
	UINT m_VertexBufferStride = 0; // 버텍스 한개의 크기
	UINT m_VertexBufferOffset = 0; // 버텍스 한개에 대한 설정
	
	UINT m_VertexCount;
	UINT m_Indices;
	void CreateShaaders();
	void CreateBuffers();
	void CreateStates();
	void CreateTexture(std::wstring filepath);
	void DrawSkyBox(ID3D11Buffer* pConstantBuffer);
};