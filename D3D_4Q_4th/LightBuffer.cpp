#include "framework.h"
#include "LightBuffer.h"
#include "../DirectX_3D_Lilbrary/helper.h"

void LightBuffer::CreateShaders()
{
	// Directional Light
	{
		ComPtr<ID3DBlob> psBlob;
		CompileShaderFromFile(L"DirectionalLightPS.hlsl", "main", "ps_5_0", psBlob.GetAddressOf());
		HR_T(m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_pDirectionalLightPS.GetAddressOf()));
	}

	// Point Light
	{
		ComPtr<ID3DBlob> vsBlob;
		CompileShaderFromFile(L"PointLightVS.hlsl", "main", "vs_5_0", vsBlob.GetAddressOf());
		HR_T(m_pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_pPointLightVS.GetAddressOf()));

		ComPtr<ID3DBlob> psBlob;
		CompileShaderFromFile(L"PointLIghtPS.hlsl", "main", "ps_5_0", psBlob.GetAddressOf());
		HR_T(m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_pPointLightPS.GetAddressOf()));
	}
}
