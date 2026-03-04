#pragma once
#include "framework.h"
class DeviceBase
{
public:
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;

	bool SetDeviceAndDeviceContext(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext) 
	{ 
		if (nullptr == pDevice || nullptr == pDeviceContext) { return false; }

		m_pDevice = pDevice; 
		m_pDeviceContext = pDeviceContext;
		return true;
	}
};

