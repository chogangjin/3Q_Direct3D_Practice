#pragma once
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace Microsoft::WRL;
//class ConstantBuffer;
class SkeletalMesh
{
public:
	SkeletalMesh();
	~SkeletalMesh();

	BoneMatrixContainer m_SkeletonPose;
	
	bool LoadModel(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* devicecontext, std::string filepath);
	void Update(float deltatime);
	void Draw(ConstantBuffer* pCBuffer, ID3D11Buffer* pConstantBuffer);
	void DrawAnimation(ConstantBuffer* pCBuffer, ID3D11Buffer* pConstantBuffer, ID3D11Buffer* pBonePoseBuffer, ID3D11Buffer* pBoneOffsetBuffer);
	void CheckAnimationType(const aiScene* pScene);
	void Close();
	std::vector<Mesh>& GetMeshes() { return m_Meshes; }
	std::vector<Mesh> m_Meshes;
	std::vector<Bone> m_Skeleton;
	SkeletonInfo m_SkeletonInfo;
	
	Matrix m_WorldMatrix = DirectX::XMMatrixIdentity();
	
	Vector3 m_Scale{ 1,1,1 };
	Vector3 m_Rotation{ 0,0,0 };
	Vector3 m_Translation{ 0,0,0 };

	bool m_IsRigid = true;
	bool m_HasAnimation = false;
private:
	HWND hwnd;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceContext;
	std::string filepath;
	
	//읽기 전용 데이터
	std::vector<Texture> m_LoadedTextures;
	std::vector<Animation> m_Animations;

	float m_AnimationProgressTime = 0.0f;
	int m_CurrentAnimationIndex = 0;

	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	void LoadAnmiation(aiAnimation* pAnim, const aiScene* scene);
	std::vector<Texture> LoadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName, const aiScene* scene);
	ID3D11ShaderResourceView* LoadEmbeddedTexture(const aiTexture* embeddedTexture);
};