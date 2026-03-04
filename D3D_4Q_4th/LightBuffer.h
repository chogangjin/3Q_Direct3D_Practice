#pragma once
#include "DeviceBase.h"

using namespace Microsoft::WRL;
class LightBuffer : public DeviceBase
{
public:
	ComPtr<ID3D11VertexShader> m_pDirectionalLightVS;
	ComPtr<ID3D11PixelShader> m_pDirectionalLightPS;
	
	ComPtr<ID3D11VertexShader> m_pPointLightVS;
	ComPtr<ID3D11PixelShader> m_pPointLightPS;

	void CreateShaders();
};

