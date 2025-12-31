#include "framework.h"
#include "IBL.h"
#include <directxtk/WICTextureLoader.h>
#include <directxtk/DDSTextureLoader.h>

void IBL::SetDeviceandDeviceContext(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
	m_pDevice = pDevice;
	m_pDeviceContext = pDeviceContext;
}

void IBL::SetIrradianceMap(const std::wstring filepath)
{
	m_IrradianceMapPath = filepath;
	Microsoft::WRL::ComPtr<ID3D11Resource> pResource;
	DirectX::CreateDDSTextureFromFile(m_pDevice, filepath.c_str(), pResource.GetAddressOf(), m_pIrradianceMapSRV.GetAddressOf());
}

void IBL::SetEnvironmentMap(const std::wstring filepath)
{
	m_EnvironmentMapPath = filepath;
	Microsoft::WRL::ComPtr<ID3D11Resource> pResource;
	DirectX::CreateDDSTextureFromFile(m_pDevice, filepath.c_str(), pResource.GetAddressOf(), m_pEnvironmentMapSRV.GetAddressOf());
}

void IBL::SetPrefilteredMap(const std::wstring filepath)
{
	m_PrefilteredMapPath = filepath;
	Microsoft::WRL::ComPtr<ID3D11Resource> pResource;
	DirectX::CreateDDSTextureFromFile(m_pDevice, filepath.c_str(), pResource.GetAddressOf(), m_pPrefilteredMapSRV.GetAddressOf());
}

void IBL::SetLookUpTexture(const std::wstring filepath)
{
	m_LookUpTexturePath = filepath;
	Microsoft::WRL::ComPtr<ID3D11Resource> pResource;
	DirectX::CreateDDSTextureFromFile(m_pDevice, filepath.c_str(), pResource.GetAddressOf(), m_pLookupDTextureSRV.GetAddressOf());
}

void IBL::LoadIBLMaterialFromFilePath(const std::wstring filename)
{
	ID3D11ShaderResourceView* nullSRV = nullptr;
	m_pDeviceContext->PSSetShaderResources(10, 1, &nullSRV);
	m_pDeviceContext->PSSetShaderResources(11, 1, &nullSRV);
	m_pDeviceContext->PSSetShaderResources(12, 1, &nullSRV);
	m_pDeviceContext->PSSetShaderResources(13, 1, &nullSRV);

	m_EnvironmentMapPath = L"../Resources/IBL/" + filename + L"EnvHDR.dds";
	m_IrradianceMapPath  = L"../Resources/IBL/" + filename + L"DiffuseHDR.dds";
	m_PrefilteredMapPath = L"../Resources/IBL/" + filename + L"SpecularHDR.dds";
	m_LookUpTexturePath  = L"../Resources/IBL/" + filename + L"Brdf.dds";

	Microsoft::WRL::ComPtr<ID3D11Resource> pResource1;
	Microsoft::WRL::ComPtr<ID3D11Resource> pResource2;
	Microsoft::WRL::ComPtr<ID3D11Resource> pResource3;
	Microsoft::WRL::ComPtr<ID3D11Resource> pResource4;

	DirectX::CreateDDSTextureFromFile(m_pDevice, m_EnvironmentMapPath.c_str(), pResource1.GetAddressOf(), m_pEnvironmentMapSRV.GetAddressOf());
	DirectX::CreateDDSTextureFromFile(m_pDevice, m_IrradianceMapPath.c_str(),  pResource2.GetAddressOf(), m_pIrradianceMapSRV .GetAddressOf());
	DirectX::CreateDDSTextureFromFile(m_pDevice, m_PrefilteredMapPath.c_str(), pResource3.GetAddressOf(), m_pPrefilteredMapSRV.GetAddressOf());
	DirectX::CreateDDSTextureFromFile(m_pDevice, m_LookUpTexturePath.c_str(),  pResource4.GetAddressOf(), m_pLookupDTextureSRV.GetAddressOf());
}

void IBL::SetIBLMaterial()
{
	//if (IBLIndex == index) return;
	//IBLIndex = index;

	if (m_IsChangeIBL == false) { return; }
	m_IsChangeIBL = false;

	if (m_IBLIndex == 1) { m_IBLIndex = 0; LoadIBLMaterialFromFilePath(L"Output"); }
	else if (m_IBLIndex == 0) { m_IBLIndex = 1; LoadIBLMaterialFromFilePath(L"Qwantani"); }
}

void IBL::SetIBLSRV()
{
	m_pDeviceContext->PSSetShaderResources(10, 1, m_pEnvironmentMapSRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(11, 1, m_pIrradianceMapSRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(12, 1, m_pPrefilteredMapSRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(13, 1, m_pLookupDTextureSRV.GetAddressOf());
}
