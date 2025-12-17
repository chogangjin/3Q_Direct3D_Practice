#pragma once
#include "framework.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Animation.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Microsoft::WRL;
struct ConstantBuffer
{
	Matrix World;		// 16
	Matrix View;		// 16
	Matrix Projection;  // 16

	Vector4 DirectionalLight;
	Vector4 DirectionalLightColor;

	Vector4 DiffuseColor;
	Vector4 AmbientColor;
	Vector4 SpecularColor;

	Vector4 DiffuseMaterial;
	Vector4 AmbientMaterial;
	Vector4 SpecularMaterial;

	Vector3 CameraPos;
	float shininess;

	bool IsRigid;
	UINT RefBoneIndex;
	Vector2 Padding;
};
struct BoneMatrixContainer
{
	Matrix modelMatrix[128]{};
};

struct TransformViewProjection
{
	Matrix ShadowView;
	Matrix ShadowProjection;
};

struct Vertex
{
	Vector3 position;
	Vector2 texture;

	//TBN Coordinate
	Vector3 Normal;
	Vector3 Tangent;
	Vector3 BiNormal;
	int BlendIndices[4] = {};
	float BlendWeights[4] = {};
	
	Vertex(){}
	Vertex(float x, float y, float z) : position(x, y, z) {}
	Vertex(Vector3 position, Vector2 texture = {}, Vector3 normal = {}, Vector3 tangent = {}, Vector3 binormal = {}) : position(position), texture(texture), Normal(normal), Tangent(tangent), BiNormal(binormal) {}
	
	void AddBoneData(int index, float weight)
	{
		assert(BlendWeights[0] == 0.0f || BlendWeights[1] == 0.0f ||
			BlendWeights[2] == 0.0f || BlendWeights[3] == 0.0f);
		for (int i = 0; i < 4; i++)
		{
			if (BlendWeights[i] == 0.0f)
			{
				BlendIndices[i] = index;
				BlendWeights[i] = weight;
				return;
			}
		}
	}
};

struct BoneWeightVertex
{
	Vector3 position;
	Vector2 texture;
	Vector3 Normal;
	Vector3 Tangent;

	int BlendIndices[4] = {};
	float BlendWeights[4] = {};

	void AddBoneData(int index, float weight)
	{
		assert(BlendWeights[0] == 0.0f || BlendWeights[0] == 0.0f ||
			BlendWeights[0] == 0.0f || BlendWeights[0] == 0.0f);
		for (int i = 0; i < 4; i++)
		{
			if (BlendWeights[i] == 0.0f)
			{
				BlendIndices[i] = index;
				BlendWeights[i] = weight;
				return;
			}
		}
	}
};

struct Texture
{
	std::string type; // 텍스쳐 타입 -> Diffuse, Normal, Specular 등등
	std::string path; // 파일을 불러올 경로
	ComPtr<ID3D11ShaderResourceView> m_pTextureSRV;
};

struct Bone
{
	int m_Index;
	int m_ParentIndex = -1;
	std::string m_Name;
	BoneAnimation* m_pBoneAnimation = nullptr;
	Matrix localMatrix = Matrix::Identity; // 메쉬에서 본으로 변환한 행렬
	Matrix modelMatrix = Matrix::Identity;  // Skinning 최종 결과
};

struct BoneInfo // ainode를 받는 생성자로 이름과 정보값 그대로 받아옴
{
	Matrix relativeMatrix;
	std::string Name;
	std::string ParentBoneName;
	int index = -1;
	BoneInfo(const aiNode* pNode)
	{
		Name = pNode->mName.C_Str();
		relativeMatrix = Matrix(&pNode->mTransformation.a1).Transpose();
	}
};

// 캐릭터 전체의 본 이름과 Relative matrix 정보 받아오고, 본에 번호 부여해줌
// map container를 순회하여 본 이름으로 번호 얻거나, 번호로 본 정보 얻어옴
struct SkeletonInfo 
{
	std::vector<BoneInfo> Bones;
	std::map<std::string, int> m_BoneMappingTable;
	std::map<std::string, int> m_MeshMappingTable;
	BoneMatrixContainer m_BoneOffsetMatrices;

	void CreatefromAiScene(const aiScene* pScene);
	void CreateBoneInfo(const aiNode* pNode, const aiScene* pScene);
	BoneInfo* GetBoneInfoByName(const std::string);
	BoneInfo* GetBoneInfoByIndex(int m_Index);
	int GetBoneIndexByName(const std::string& name);
	int GetBoneIndexByMeshName(const std::string& name);
	const std::string GetBoneName(int m_Index);
	bool IsExist(const std::string& name);
};

class Mesh
{
public:
	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ComPtr<ID3D11Buffer> m_pBoneWeightVertexBuffer;
	ComPtr<ID3D11Buffer> m_pIndexBuffer;
	ID3D11Device* m_pDevice;
	std::vector<Vertex> m_Vertices;
	std::vector<BoneWeightVertex> m_BoneWeightVertices;
	std::vector<UINT> m_Indices;
	std::vector<Texture> m_Textures;
	int m_RefBoneIndex;
	ConstantBuffer* cbuffer = nullptr;
	ID3D11Buffer* m_pConstantBuffer = nullptr;
	ID3D11Buffer* m_pBoneMatrixBuffer = nullptr;

	Mesh() = default;
	Mesh(ID3D11Device* device, const std::vector<Vertex>& vertices, const std::vector<UINT>& indices, const std::vector<Texture>& textures) :
		m_pDevice(device),
		m_Vertices(vertices),
		m_Indices(indices),
		m_Textures(textures)
	{
		CreateVertexBuffer(device);
		CreateIndexBuffer(device);
	}
	
	Mesh(ID3D11Device* device, const std::vector<BoneWeightVertex>& boneWeightVertices, const std::vector<UINT>& indices, const std::vector<Texture>& textures) :
		m_pDevice(device),
		m_BoneWeightVertices(boneWeightVertices),
		m_Indices(indices),
		m_Textures(textures)
	{
// 		CreateWeightedVertexBuffer(device);
// 		CreateIndexBuffer(device);
	}


	void Update(float deltaTime)
	{

	}

	void DrawMesh(ID3D11DeviceContext* deviceContext)
	{
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);	// 버텍스 버퍼 입력 조립
		deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// 인덱스 버퍼 입력 조립
		for (UINT i = 0; i < m_Textures.size(); i++)
		{
			// diffuse
			if (m_Textures[i].type == "texture_diffuse")
			{
				deviceContext->PSSetShaderResources(0, 1, m_Textures[i].m_pTextureSRV.GetAddressOf()); // SRV 벡터의 첫번째 주소를 전달해서 쭉 읽도록
			}

			// specular
			if (m_Textures[i].type == "texture_specular")
			{
				deviceContext->PSSetShaderResources(3, 1, m_Textures[i].m_pTextureSRV.GetAddressOf());
			}

			// ambient
			if (m_Textures[i].type == "texture_ambient")
			{
				deviceContext->PSSetShaderResources(4, 1, m_Textures[i].m_pTextureSRV.GetAddressOf());
			}

			// emissive
			if (m_Textures[i].type == "texture_emissive")
			{
				deviceContext->PSSetShaderResources(5, 1, m_Textures[i].m_pTextureSRV.GetAddressOf());
			}
			
			// opacity
			deviceContext->PSSetShaderResources(6, 1, m_Textures[i].m_pTextureSRV.GetAddressOf());
			
		}
		deviceContext->DrawIndexed(static_cast<UINT>(m_Indices.size()), 0, 0);						// 인덱스 버퍼에 저장되어있는 인덱스대로 그리기
	}

public:
	void CreateVertexBuffer(ID3D11Device* device)
	{
		HRESULT hr;

		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;			// GPU에서만 읽을 수 있고, cpu에서 엑세스 x, 작성 또는 변경 불가
		vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * m_Vertices.size());
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;  // 버텍스 버퍼로 사용
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags;								// mip-map설정 관련 플래그

		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &m_Vertices[0]; // 주소 넘겨서 버텍스 정보를 벡터 첫번째부터 쭉 읽게 함

		hr = device->CreateBuffer(&vertexBufferDesc, &BufferData, &m_pVertexBuffer); // 버텍스 버퍼 생성
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to Create Vertex Buffer!");
		}
	}

	void CreateIndexBuffer(ID3D11Device* pDevice)
	{
		D3D11_BUFFER_DESC indexBufferDesc = {};
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(UINT) * m_Indices.size());
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		
		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &m_Indices[0];

		HRESULT hr = pDevice->CreateBuffer(&indexBufferDesc, &BufferData, &m_pIndexBuffer);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to Create Index Buffer!");
		}
	}

	void CreateWeightedVertexBuffer(ID3D11Device* pDevice, const std::vector<BoneWeightVertex>& boneweightvertex )
	{
		D3D11_BUFFER_DESC wvBuffer = {};
		wvBuffer.Usage = D3D11_USAGE_IMMUTABLE;
		wvBuffer.ByteWidth = static_cast<UINT>(sizeof(BoneWeightVertex)) * static_cast<UINT>(boneweightvertex.size());
		wvBuffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
		wvBuffer.CPUAccessFlags = 0;
		wvBuffer.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA BufferData = {};
		BufferData.pSysMem = &m_BoneWeightVertices[0];

		HRESULT hr = pDevice->CreateBuffer(&wvBuffer, &BufferData, &m_pBoneWeightVertexBuffer);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to Create Weight Vertex Buffer!");
		}
	}
};
