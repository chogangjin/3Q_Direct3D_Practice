#include "BillBoard.h"
#include "framework.h"
#include <directxtk/WICTextureLoader.h>

using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;
using namespace DirectX;

// TODO : 리소스 재활용 할 수 있도록 해야 함, Need Resource Recycle
// TODO : 컴포넌트 화 시켜서 한번에 나올 수 있도록 해주어야 함, Should to be Component to render all at once

struct BillBoardVertex
{
	Vector3 Position;
	Vector4 Color;
	Vector2 UV;
};

struct BillBoardConstantBuffer
{
	Matrix BillBoardWorld;
	Matrix BillBoardView;
	Matrix BillBoardProjection;

	Vector2 uvScale;
	Vector2 uvOffset;
};

void BillBoard::CreateShaders()
{
	ComPtr<ID3DBlob> vsBlob;
	HR_T(CompileShaderFromFile(L"BillBoardVertexShader.hlsl", "main", "vs_5_0", vsBlob.GetAddressOf()));
	HR_T(m_pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	HR_T(m_pDevice->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), m_pInputLayout.GetAddressOf()));

	ComPtr<ID3DBlob> psBlob;
	HR_T(CompileShaderFromFile(L"BillBoardPixelShader.hlsl", "main", "ps_5_0", psBlob.GetAddressOf()));
	HR_T(m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));
}

void BillBoard::CreateBuffers()
{
	//버텍스 버퍼 생성
	BillBoardVertex vertices[] =
	{
		{Vector3{-0.5f,  0.5f, 0.0f}, Vector4{1.0f, 0.0f, 0.0f, 1.0f},Vector2(0,0) },
		{Vector3{ 0.5f,  0.5f, 0.0f}, Vector4{0.0f, 1.0f, 0.0f, 1.0f},Vector2(1,0) },
		{Vector3{ 0.5f, -0.5f, 0.0f}, Vector4{0.0f, 0.0f, 1.0f, 1.0f},Vector2(1,1) },
		{Vector3{-0.5f, -0.5f, 0.0f}, Vector4{1.0f, 1.0f, 0.0f, 1.0f},Vector2(0,1) }
	};

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	HR_T(m_pDevice->CreateBuffer(&bufferDesc, &initData, m_pVertexBuffer.GetAddressOf()));

	//인덱스 버퍼 생성
	UINT indices[] =
	{
		0,1,2,
		0,2,3
	};

	bufferDesc.ByteWidth = sizeof(indices);
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	initData.pSysMem = indices;
	HR_T(m_pDevice->CreateBuffer(&bufferDesc, &initData, m_pIndexBuffer.GetAddressOf()));

	// 상수 버퍼 생성
	bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(BillBoardConstantBuffer);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HR_T(m_pDevice->CreateBuffer(&bufferDesc, nullptr, m_pConstantBuffer.GetAddressOf()));
}

void BillBoard::CreateState()
{
	D3D11_RENDER_TARGET_BLEND_DESC billboardBlendDesc = {};
	billboardBlendDesc.BlendEnable = true;
	billboardBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	billboardBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	billboardBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	billboardBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	billboardBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;	
	billboardBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	billboardBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = true;
	blendDesc.RenderTarget[0] = billboardBlendDesc;

	HR_T(m_pDevice->CreateBlendState(&blendDesc, m_pBlendState.GetAddressOf()));
}

void BillBoard::Update(float DeltaTime)
{
	m_pCamera->GetViewMatrix(m_View);
	const Vector3 billboardPos = m_Position;
	const float dt = DeltaTime;
	if (dt > 0.000001f)
	{
		m_BillBoardVelocity = (billboardPos - m_BillBoardPrevPosition) / dt;
	}
	else
	{
		m_BillBoardVelocity = Vector3::Zero;
	}
	m_BillBoardPrevPosition = billboardPos;

	switch (m_type)
	{
	case BillboardType::Identity:
	{
		m_Matrix = Matrix::Identity;
		break;
	}

	case BillboardType::YAxisLocked:
	{
		Vector3 cameraPos = m_pCamera->m_Position;

		Vector3 forward = billboardPos - cameraPos;
		forward.y = 0.0f;

		if (forward.LengthSquared() < 0.0001f)
		{
			forward = Vector3::Forward;
		}
		else
		{
			forward.Normalize();
		}

		Vector3 up = Vector3::Up;
		Vector3 right = up.Cross(forward);
		right.Normalize();

		m_Matrix = Matrix::Identity;
		m_Matrix._11 = right.x;
		m_Matrix._12 = right.y;
		m_Matrix._13 = right.z;
		m_Matrix._21 = up.x;
		m_Matrix._22 = up.y;
		m_Matrix._23 = up.z;
		m_Matrix._31 = forward.x;
		m_Matrix._32 = forward.y;
		m_Matrix._33 = forward.z;
		break;
	}

	case BillboardType::Spherical:
	{
		Vector3 cameraPos = m_pCamera->m_Position;
		Vector3 forward = billboardPos - cameraPos;
		
		if (forward.LengthSquared() < 0.0001f) { forward = Vector3::Forward; }
		else { forward.Normalize(); }

		Vector3 up = Vector3::Up;

		if (abs(forward.Dot(up)) > 0.999f) { up = Vector3::Right; }

		Vector3 right = up.Cross(forward);
		right.Normalize();
		up = forward.Cross(right);
		up.Normalize();

		m_Matrix = Matrix::Identity;
		m_Matrix._11 = right.x;
		m_Matrix._12 = right.y;
		m_Matrix._13 = right.z;
		m_Matrix._21 = up.x;
		m_Matrix._22 = up.y;
		m_Matrix._23 = up.z;
		m_Matrix._31 = forward.x;
		m_Matrix._32 = forward.y;
		m_Matrix._33 = forward.z;
		break;
	}

	case BillboardType::ScreenAligned:
	{
		Matrix viewInverse = m_View.Invert();
		m_Matrix = Matrix::Identity;
		m_Matrix._11 = viewInverse._11;
		m_Matrix._12 = viewInverse._12;
		m_Matrix._13 = viewInverse._13;
		m_Matrix._21 = viewInverse._21;
		m_Matrix._22 = viewInverse._22;
		m_Matrix._23 = viewInverse._23;
		m_Matrix._31 = viewInverse._31;
		m_Matrix._32 = viewInverse._32;
		m_Matrix._33 = viewInverse._33;
		break;
	}
	}

	m_World = Matrix::CreateScale(m_Scale.x, m_Scale.y, m_Scale.z) * m_Matrix * Matrix::CreateTranslation(m_Position);
	
	if(m_HasAnimation)
	{
		m_AnimTime += DeltaTime;
		if (m_AnimTime >= FPS)
		{
			m_CurrentFrame++;
			m_AnimTime = fmod(m_AnimTime, FPS);
			
			if(m_AnimationType == AnimType::Loop)
			{
				if (m_CurrentFrame > m_FrameCount)
				{
					m_CurrentFrame = 0;
				}
				m_IsFinished = false;
			}
			else
			{
				if(m_CurrentFrame>=m_FrameCount)
				{
					m_IsFinished = true;
				}
			}
		}

		m_UVScale = { (float)1.0 / m_Width, (float)1.0 / m_Height };
		int frameX = m_CurrentFrame % m_Width;
		int frameY = m_CurrentFrame / m_Width;
		m_UVOffset = { frameX * m_UVScale.x, frameY * m_UVScale.y };
	}
}

void BillBoard::Render()
{
	UINT stride = sizeof(BillBoardVertex);
	UINT offset = 0;

	BillBoardConstantBuffer cb;
	cb.BillBoardWorld = m_World.Transpose();
	cb.BillBoardView = m_View.Transpose();
	cb.BillBoardProjection = m_Projection.Transpose();
	cb.uvScale = m_UVScale;
	cb.uvOffset = m_UVOffset;

	m_pDeviceContext->OMSetBlendState(m_pBlendState.Get(), nullptr, 0xffffffff);

	m_pDeviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout.Get());
	
	m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
	m_pDeviceContext->VSSetConstantBuffers(7, 1, m_pConstantBuffer.GetAddressOf());
	m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
	m_pDeviceContext->PSSetShaderResources(19, 1, m_pShaderResourceView.GetAddressOf());
	m_pDeviceContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);
	
	if(!m_IsFinished)
	{
		m_pDeviceContext->DrawIndexed(6, 0, 0);
	}

	m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void BillBoard::SetImage(std::wstring filepath)
{
	HR_T(CreateWICTextureFromFile(m_pDevice, m_pDeviceContext, filepath.c_str(), nullptr, m_pShaderResourceView.GetAddressOf()));
}

void BillBoard::SetAnimInfo(int flag)
{
	m_Width = 6;
	m_Height = 6;
	m_FrameCount = m_Width * m_Height;
	FPS = 0.03f;
	m_HasAnimation = true;
	m_AnimationType = (AnimType)flag;
}

