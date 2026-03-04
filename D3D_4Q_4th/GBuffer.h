#pragma once
#include "DeviceBase.h"
using namespace Microsoft::WRL;

// Geometry Buffer
class GBuffer : public DeviceBase
{
public:
	// Deffered Rendering G_Buffer
	static const int m_Count = 3;
	ComPtr<ID3D11Texture2D> m_pGTextures[m_Count] = {};
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView[m_Count] = {};
	ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView[m_Count] = {};

	ComPtr<ID3D11InputLayout> m_pInputLayout;
	ComPtr<ID3D11VertexShader> m_pVertexShader;
	ComPtr<ID3D11PixelShader> m_pPixelShader;

	bool CreateGBuffer(float width, float height);
	bool CreateGBufferShaders();
};

