#pragma once
#include "framework.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Microsoft::WRL;


class HDR_Quad
{
public:
	HDR_Quad() {}
	HDR_Quad(ID3D11Device* pDevice) : m_pDevice(pDevice) {}
	
	ID3D11Device* m_pDevice;
	ComPtr<ID3D11Buffer> m_pQuadVertexBuffer = nullptr;
	ComPtr<ID3D11InputLayout> m_pQuadInputLayout = nullptr;
	ComPtr<ID3D11VertexShader> m_pQuadVertexShader = nullptr;
	ComPtr<ID3D11Buffer> m_pQuadIndexBuffer = nullptr;

	ComPtr<ID3D11PixelShader> m_pHDRPixelShader = nullptr;
	ComPtr<ID3D11PixelShader> m_pLDRPixelShader = nullptr;

	UINT m_QuadVertexBufferStride = 0;
	UINT m_QuadVertexBufferOffset = 0;
	int m_NumQuadIndices = 0;

	void SetDevice(ID3D11Device* pDevice) { m_pDevice = pDevice; }
	void CreateHDRQuad();
	void CreateQuadVertexShader();
	void CreateHDRandLDRPixelShader();
	//void DrawHDRQuad(ID3D11DeviceContext* pDeviceContext, DXGI_FORMAT format, ID3D11ShaderResourceView* pShaderResourceView);
	void DrawHDRQuad(ID3D11DeviceContext* pDeviceContext, DXGI_FORMAT format, ID3D11ShaderResourceView* pShaderResourceView, ID3D11SamplerState* pSamplerState);
}; 

