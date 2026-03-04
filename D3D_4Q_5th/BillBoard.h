#pragma once
#include "DeviceBase.h"
#include "../DirectX_3D_Lilbrary/Helper.h"
#include "../DirectX_3D_Lilbrary/Camera.h"
using namespace Microsoft::WRL;

enum class BillboardType
{
	Identity = 0,
	YAxisLocked = 1,
	Spherical = 2,
	ScreenAligned = 3
};

enum class AnimType
{
	None = 0,
	NonLoop = 1,
	Loop = 2
};

class BillBoard : public DeviceBase
{
public:
	ComPtr<ID3D11InputLayout> m_pInputLayout;
	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ComPtr<ID3D11Buffer> m_pIndexBuffer;
	ComPtr<ID3D11Buffer> m_pConstantBuffer;
	
	Camera* m_pCamera = nullptr;

	ComPtr<ID3D11VertexShader> m_pVertexShader;
	ComPtr<ID3D11PixelShader> m_pPixelShader;

	ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView;
	ComPtr<ID3D11BlendState> m_pBlendState;

	Vector3 m_BillBoardCurrPosition = {};
	Vector3 m_BillBoardPrevPosition = {};
	Vector3 m_BillBoardVelocity = {};

	Vector3 m_Position = {0, 100, -100};
	Vector3 m_Rotation = { 0,0,0 };
	Vector3 m_Scale = { 100,100,100 };

	Matrix m_Matrix = Matrix::Identity;
	Matrix m_World = Matrix::Identity;
	Matrix m_View = Matrix::Identity;
	Matrix m_Projection = Matrix::Identity;
	
	BillboardType m_type = BillboardType::Identity;

	bool m_HasAnimation = false;
	bool m_IsFinished = false;
	int m_Width;
	int m_Height;
	int m_FrameCount = 0;
	float FPS;
	int m_CurrentFrame = 0;
	float m_AnimTime = 0.0f;
	Vector2 m_UVScale;
	Vector2 m_UVOffset;
	AnimType m_AnimationType = AnimType::None;

	void SetCamera(Camera* pCamera) { m_pCamera = pCamera; }
	void CreateShaders();
	void CreateBuffers();
	void CreateState();
	void Update(float DeltaTime);
	void Render();
	void SetImage(std::wstring filepath);
	void SetAnimInfo(int flag);
};

