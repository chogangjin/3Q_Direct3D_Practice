#pragma once
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace Microsoft::WRL;

class StaticFBX
{
public:
	StaticFBX();
	~StaticFBX();
	
	Matrix m_WorldMatrix = DirectX::XMMatrixIdentity();

	Vector3 m_Scale{ 1,1,1 };
	Vector3 m_Rotation{ 0,0,0 };
	Vector3 m_Translation{ 0,0,0 };
	bool m_HasNormalMap = false;

	bool LoadModel(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* devicecontext, std::string filepath);
	void Update();
	void Draw(ConstantBuffer* pCBuffer, ID3D11Buffer* pConstantBuffer);
	void Close();
private:
	HWND hwnd;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	std::vector<Mesh> m_meshes;
	std::string filepath;
	std::vector<Texture> textures_loaded;

	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName, const aiScene* scene);
	ID3D11ShaderResourceView* loadEmbeddedTexture(const aiTexture* embeddedTexture);
};

