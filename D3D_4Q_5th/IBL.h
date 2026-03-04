#pragma once

class IBL
{
private:
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pDeviceContext = nullptr;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pEnvironmentMapSRV = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pIrradianceMapSRV = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pPrefilteredMapSRV = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pLookupDTextureSRV = nullptr;
	
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_pvEnvironmentMapSRV;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_pvIrradianceMapSRV;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_pvPrefilteredMapSRV;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_pvLookupDTextureSRV;

public:
	std::wstring m_EnvironmentMapPath;
	std::wstring m_IrradianceMapPath;
	std::wstring m_PrefilteredMapPath;
	std::wstring m_LookUpTexturePath;
	//UINT m_MaxMipLevel = 0;
	UINT m_IBLIndex = 0;
	bool m_IsChangeIBL = false;

	void SetDevice(ID3D11Device* pDevice) { m_pDevice = pDevice; }
	void SetDeviceContext(ID3D11DeviceContext* pDeviceContext) { m_pDeviceContext = pDeviceContext; }
	void SetDeviceandDeviceContext(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	void LoadIBLMaterialFromFilePath(const std::wstring filepath);
	void SetIBLMaterial();
	void SetIBLSRV();
};

