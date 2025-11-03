#include "framework.h"
#include "ModelLoader.h"
#include <directxtk/WICTextureLoader.h>
#include <filesystem>
#include "../DirectX_3D_Lilbrary/TimeSystem.h"

SkeletalMesh::SkeletalMesh() :
	m_pDevice(nullptr), m_pDeviceContext(nullptr), m_Meshes(), filepath(),
	m_LoadedTextures(), hwnd(nullptr)
{
}

SkeletalMesh::~SkeletalMesh()
{
}

bool SkeletalMesh::LoadModel(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* devicecontext, std::string _filepath)
{
	//Assimp 임포트 옵션
	Assimp::Importer importer = {};
	unsigned int importFlags = aiProcess_Triangulate | // vertex를 삼각형으로 출력
	aiProcess_GenNormals | // Normal 생성
	aiProcess_GenUVCoords | // UV 좌표 생성
	aiProcess_CalcTangentSpace | // 탄젠트 벡터 생성
	aiProcess_ConvertToLeftHanded; // 왼손 좌표계 
	//aiProcess_PreTransformVertices; // 노드의 변환행렬을 미리 적용 -> StaticMesh로 처리할때만 사용 -> animation에서 미사용
	
	const aiScene* pScene = importer.ReadFile(_filepath, importFlags); // 파일 정보를 aiscene에 담음
	
	if(pScene == nullptr)
	{
		return false;
	}

	//device, devicecontext, filepath, hwnd를 그대로 넘겨줌
	this->filepath = _filepath.substr(0, _filepath.find_last_of("/\\"));
	this->m_pDevice = device;
	this->m_pDeviceContext = devicecontext;
	this->hwnd = hwnd;
	
	//스켈레톤 정보 읽고, 그걸 토대로 가지고 놀기
	m_SkeletonInfo.CreatefromAiScene(pScene);

	//애니메이션을 먼저 읽고, 노드를 재귀방식으로 연결
	if (pScene->HasAnimations())
	{
		for (UINT i = 0; i < pScene->mNumAnimations; i++)
		{
			LoadAnmiation(pScene->mAnimations[i], pScene);
		}
	}
	
	ProcessNode(pScene->mRootNode, pScene);

	return true;
}

void SkeletalMesh::Update(float deltatime)
{
	if (!m_Animations.empty())
	{
		m_AnimationProgressTime += deltatime;
		m_AnimationProgressTime = fmod(m_AnimationProgressTime, m_Animations[m_CurrentAnimationIndex].m_Duration);
	}
	std::cout << m_AnimationProgressTime << std::endl;
	for (auto& bone : m_Skeleton)
	{
		if (bone.m_pBoneAnimation != nullptr)
		{
			Vector3 position;
			Quaternion rotation;
			Vector3 scale;

			// 델타타임에 따른 트랜스폼 값 보간
			bone.m_pBoneAnimation->Evaluate(m_AnimationProgressTime, position, rotation, scale);
			bone.localMatrix = Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation)  * Matrix::CreateTranslation(position);
		}
		
		if (bone.ParentIndex != -1 && bone.index != 0)
		{
			bone.modelMatrix = bone.localMatrix * m_Skeleton[bone.ParentIndex].modelMatrix;
		}
		else if(bone.index == 0)
		{
			bone.modelMatrix = bone.localMatrix;
		}

		//GPU에 전달할 정보
		m_SkeletonPose.modelMatrix[bone.index] = (bone.modelMatrix).Transpose();
	}
}

void SkeletalMesh::Draw(ID3D11DeviceContext* devicecontext)
{
	for (size_t i = 0; i < m_Meshes.size(); ++i)
	{
		m_Meshes[i].DrawMesh(devicecontext);
	}
}

void SkeletalMesh::Close()
{

}

void SkeletalMesh::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		aiNode* pChildNode = node->mChildren[i];
		Bone bone;
		bone.name = pChildNode->mName.C_Str();
		bone.localMatrix = Matrix{ &node->mChildren[i]->mTransformation.a1 }.Transpose();
		bone.index = m_Skeleton.size();
		if (node == scene->mRootNode)
		{
			//bone.index = 0;
			bone.ParentIndex = -1;
			m_SkeletonInfo.m_BoneMappingTable.insert({ bone.name, bone.index });
			m_Skeleton.push_back(bone);
			this->ProcessNode(node->mChildren[i], scene);
			continue;
		} 
		else
		{
			bone.ParentIndex = m_SkeletonInfo.GetBoneIndexByName(m_SkeletonInfo.Bones[bone.index].ParentBoneName);
			m_SkeletonInfo.m_BoneMappingTable.insert({ bone.name, bone.index });
			
			//메쉬 받아오기
			for (UINT j = 0; j < node->mNumMeshes; ++j)
			{
				// 노드의 mMesh는 scene의 mMesh인덱스 번호를 반환
				aiMesh* pMesh = scene->mMeshes[pChildNode->mMeshes[j]];
				m_Meshes.push_back(this->ProcessMesh(pMesh, scene));
				m_Meshes.back().m_RefBoneIndex = bone.index;
				m_SkeletonInfo.m_MeshMappingTable.insert({pMesh->mName.C_Str(), bone.index});
			}

			for (auto& it : m_Animations[0].m_BoneAnimations)
			{
				if (it.name == bone.name)
				{
					bone.m_pBoneAnimation = &it;
				}
			}

			m_Skeleton.push_back(bone);
			this->ProcessNode(node->mChildren[i], scene);
		}
	}
}

Mesh SkeletalMesh::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices; // 메쉬에 들어갈 정점 정보
	std::vector<UINT> indices; // 인덱스 버퍼에 들어갈 정점의 인덱스
	std::vector<Texture> textures; // 메쉬에 들어갈 텍스쳐들
	for (UINT i = 0; i < mesh->mNumVertices; ++i) // 버텍스 정보 입력
	{
		Vertex vertex{
			mesh->mVertices[i].x, 
			mesh->mVertices[i].y, 
			mesh->mVertices[i].z,
		};

		if (mesh->mTextureCoords[0])
		{
			vertex.texture.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.texture.y = (float)mesh->mTextureCoords[0][i].y;
		}

		if (mesh->HasNormals())
		{
			vertex.Normal.x = mesh->mNormals[i].x;
			vertex.Normal.y = mesh->mNormals[i].y;
			vertex.Normal.z = mesh->mNormals[i].z;
		}

		if (mesh->HasTangentsAndBitangents())
		{
			vertex.Tangent.x = mesh->mTangents[i].x;
			vertex.Tangent.y = mesh->mTangents[i].y;
			vertex.Tangent.z = mesh->mTangents[i].z;

			vertex.BiNormal.x = mesh->mBitangents[i].x;
			vertex.BiNormal.y = mesh->mBitangents[i].y;
			vertex.BiNormal.z = mesh->mBitangents[i].z;
		}
		vertices.push_back(vertex);
	}

	// 인덱스로 만들어진 폴리곤 면들
	for (UINT i = 0; i < mesh->mNumFaces; ++i)		// face 정보 입력
	{
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++) // face가 가지고 있는 인덱스 번호 입력
		{
			indices.push_back(face.mIndices[j]);
		}
	}
	
	if (mesh->mMaterialIndex >= 0) // 머티리얼이 존재한다면
	{
		aiMaterial* pMaterial = scene->mMaterials[mesh->mMaterialIndex];
		std::string aistr = pMaterial->GetName().C_Str();
		aiString astr;

		// 머티리얼 타입별 적용, 실패하면 nullptr 반환
		if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &astr) == AI_SUCCESS)
		{
			for (int i = 0; i < pMaterial->GetTextureCount(aiTextureType_DIFFUSE); i++)
			{
				std::vector<Texture> Maps = this->LoadMaterialTextures(pMaterial, aiTextureType_DIFFUSE, "texture_diffuse", scene);
				textures.insert(textures.end(), Maps.begin(), Maps.end());
			}
		}
		
		if (pMaterial->GetTexture(aiTextureType_AMBIENT, 0, &astr) == AI_SUCCESS)
		{
			for (int i = 0; i < pMaterial->GetTextureCount(aiTextureType_AMBIENT); i++)
			{
				std::vector<Texture> Maps = this->LoadMaterialTextures(pMaterial, aiTextureType_AMBIENT, "texture_ambient", scene);
				textures.insert(textures.end(), Maps.begin(), Maps.end());
			}
		}
		
		if (pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &astr) == AI_SUCCESS)
		{
			for (int i = 0; i < pMaterial->GetTextureCount(aiTextureType_SPECULAR); i++)
			{
				std::vector<Texture> Maps = this->LoadMaterialTextures(pMaterial, aiTextureType_SPECULAR, "texture_specular", scene);
				textures.insert(textures.end(), Maps.begin(), Maps.end());
			}
		}
		
		if (pMaterial->GetTexture(aiTextureType_OPACITY, 0, &astr) == AI_SUCCESS)
		{
			for (int i = 0; i < pMaterial->GetTextureCount(aiTextureType_OPACITY); i++)
			{
				std::vector<Texture> Maps = this->LoadMaterialTextures(pMaterial, aiTextureType_OPACITY, "texture_opacity", scene);
				textures.insert(textures.end(), Maps.begin(), Maps.end());
			}
		}
		//else
		//{
		//	HRESULT hr;
		//	Texture t = { "texture_opacity","../Resources/Default_Material.png", nullptr };
		//	std::wstring filepath = std::wstring{ t.path.begin(), t.path.end() };
		//	hr = DirectX::CreateWICTextureFromFile(m_pDevice, m_pDeviceContext, filepath.c_str(), nullptr, t.m_pTextureSRV.GetAddressOf());
		//	if (FAILED(hr))
		//	{
		//		std::runtime_error("Texture couldn't be loaded");
		//	}
		//	textures.push_back(t);
		//}
			
		if (pMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &astr) == AI_SUCCESS)
		{
			for (int i = 0; i < pMaterial->GetTextureCount(aiTextureType_EMISSIVE); i++)
			{
				std::vector<Texture> Maps = this->LoadMaterialTextures(pMaterial, aiTextureType_EMISSIVE, "texture_emissive", scene);
				textures.insert(textures.end(), Maps.begin(), Maps.end());
			}
		}
	}
	return Mesh(m_pDevice, vertices, indices, textures);
}

//void ModelLoader::processBone(aiNode* bone, const aiScene* scene)
//{
//
//	// TODO : bone의 부모행렬 곱하는 것은 update때 실행
//	// nodeanim과 bone을 연결해서 곱해줌
//	aiNode* parent = bone->mParent;
//	aiNodeAnim* nodeanim; 
//
//	BoneInfo boneinfo(bone);
//	Bone tempbone;
//	tempbone.name = bone->mName.C_Str();
//	aiMatrix4x4 mat = bone->mTransformation;
//	tempbone.localMatrix = Matrix{ &bone->mTransformation.a1 }.Transpose();
//
//	if (bone->mParent != nullptr)
//	{
//		aiMatrix4x4 world = mat * parent->mTransformation;
//		tempbone.worldMatrix = Matrix{ &world.a1 };
//	}
//	else
//	{
//		tempbone.worldMatrix = tempbone.localMatrix;
//	}
//	tempbone.index = m_Bones.size();
//	m_Bones.push_back(tempbone);
//}

void SkeletalMesh::CreateSkeleton()
{
}

void SkeletalMesh::LoadAnmiation(aiAnimation* pAnim, const aiScene* scene)
{
		Animation anim;
		anim.m_Name = pAnim->mName.C_Str();
		anim.m_Duration = pAnim->mDuration/ pAnim->mTicksPerSecond;

		for (UINT j = 0; j < pAnim->mNumChannels; j++)
		{
			aiNodeAnim* pNodeAnim = pAnim->mChannels[j];
			BoneAnimation boneanim;
			boneanim.name = pNodeAnim->mNodeName.C_Str();

			for (UINT k = 0; k < pNodeAnim->mNumPositionKeys; k++)
			{
				KeyFrame keyframe;
				keyframe.Position.x = pNodeAnim->mPositionKeys[k].mValue.x;
				keyframe.Position.y = pNodeAnim->mPositionKeys[k].mValue.y;
				keyframe.Position.z = pNodeAnim->mPositionKeys[k].mValue.z;
				keyframe.Time = pNodeAnim->mPositionKeys[k].mTime/pAnim->mTicksPerSecond;
				boneanim.AnimationKeys.push_back(keyframe);
			}			
			
			for (UINT k = 0; k < pNodeAnim->mNumRotationKeys; k++) // 같은 키프레임이라는 전제 하에
			{
				KeyFrame& keyframe = boneanim.AnimationKeys[k];
				keyframe.Rotation.x = pNodeAnim->mRotationKeys[k].mValue.x;
				keyframe.Rotation.y = pNodeAnim->mRotationKeys[k].mValue.y;
				keyframe.Rotation.z = pNodeAnim->mRotationKeys[k].mValue.z;
				keyframe.Rotation.w = pNodeAnim->mRotationKeys[k].mValue.w;
			}			
			
			for (UINT k = 0; k < pNodeAnim->mNumScalingKeys; k++)  // 같은 키프레임이라는 전제 하에
			{
				KeyFrame& keyframe = boneanim.AnimationKeys[k];
				keyframe.Scale.x = pNodeAnim->mScalingKeys[k].mValue.x;
				keyframe.Scale.y = pNodeAnim->mScalingKeys[k].mValue.y;
				keyframe.Scale.z = pNodeAnim->mScalingKeys[k].mValue.z;
			}
			
			anim.m_BoneAnimations.push_back(boneanim);
		}
		m_Animations.push_back(anim);
}

std::vector<Texture> SkeletalMesh::LoadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName, const aiScene* scene)
{
	std::vector<Texture> textures;
	for (UINT i = 0; i < material->GetTextureCount(type); i++) //텍스쳐 개수만큼 반복
	{
		aiString str;
		material->GetTexture(type, i, &str);
		bool skip = false;
		std::string filename = std::string(str.C_Str());

		for (UINT j = 0; j < m_LoadedTextures.size(); j++)
		{
			if (std::strcmp(m_LoadedTextures[j].path.c_str(), str.C_Str()) == 0)
			{
				textures.push_back(m_LoadedTextures[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{
			HRESULT hr;
			Texture texture;
			const aiTexture* embeddedtexture = scene->GetEmbeddedTexture(str.C_Str());
			if(embeddedtexture != nullptr)
			{
				texture.m_pTextureSRV = LoadEmbeddedTexture(embeddedtexture);
			}
			else
			{
				std::string fixedFilePath = filename.substr(filename.find_last_of("\\\\")+1, filename.size());
				filename = filepath + '/' + fixedFilePath;
				std::wstring wfilename = std::wstring(filename.begin(), filename.end());
				hr = DirectX::CreateWICTextureFromFile(m_pDevice, m_pDeviceContext, wfilename.c_str(), nullptr, texture.m_pTextureSRV.GetAddressOf());
				if (FAILED(hr))
				{
					std::runtime_error("Texture couldn't be loaded");
				}
			}
			texture.type = typeName;
			texture.path = filename;
			textures.push_back(texture);
			this->m_LoadedTextures.push_back(texture);
		}
	}
	return textures;
}

ID3D11ShaderResourceView* SkeletalMesh::LoadEmbeddedTexture(const aiTexture* embeddedTexture)
{
	HRESULT hr;
	ID3D11ShaderResourceView* texture = nullptr;

	if (embeddedTexture->mHeight != 0)
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = embeddedTexture->mWidth;
		desc.Height = embeddedTexture->mHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1; 
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA subresourcedata = {};
		subresourcedata.pSysMem = embeddedTexture->pcData;
		subresourcedata.SysMemPitch = embeddedTexture->mWidth * 4;
		subresourcedata.SysMemSlicePitch = embeddedTexture->mWidth * embeddedTexture->mHeight * 4;

		ID3D11Texture2D* texture2D = nullptr;
		hr = m_pDevice->CreateTexture2D(&desc, &subresourcedata, &texture2D);
		if (FAILED(hr))
		{
			std::runtime_error("CreatedTextture2D Failed!");
		}

		hr = m_pDevice->CreateShaderResourceView(texture2D, nullptr, &texture);
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateShaderResourceView Failed!");
		}

		return texture;
	}

	const size_t size = embeddedTexture->mWidth;

	hr = CreateWICTextureFromMemory(m_pDevice, m_pDeviceContext, reinterpret_cast<const unsigned char*>(embeddedTexture->pcData), size, nullptr, &texture);
	if (FAILED(hr))
	{
		throw std::runtime_error("Texture couldn't be created from memory!");
	}

	return texture;
}
