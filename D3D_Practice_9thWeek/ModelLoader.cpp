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
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0); // assimp의 불필요한 bone을 제거해주는 함수
	unsigned int importFlags = aiProcess_Triangulate | // vertex를 삼각형으로 출력
	aiProcess_GenNormals | // Normal 생성
	aiProcess_GenUVCoords | // UV 좌표 생성
	aiProcess_CalcTangentSpace | // 탄젠트 벡터 생성
	aiProcess_LimitBoneWeights| // 본의 영향을 받는 정점의 최대 개수를 4개로 제한
	aiProcess_ConvertToLeftHanded |// 왼손 좌표계 
	aiProcessPreset_TargetRealtime_Fast;
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
		m_HasAnimation = true;
	}

	CheckAnimationType(pScene);

	ProcessNode(pScene->mRootNode, pScene);

	if(!m_IsRigid)
	{
		for (auto& mesh : m_Meshes)
		{
			for (auto& vertex : mesh.m_Vertices)
			{
				float total = 0.f;
				for (auto& weight : vertex.BlendWeights)
				{
					total += weight;
				}

				if (total > 1.f)
				{
					int a = 0;
				}
			}
		}
	}

	return true;
}

void SkeletalMesh::Update(float deltatime)
{
	//deltatime = 0;
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(m_Translation.x, m_Translation.y, m_Translation.z);
	DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	m_WorldMatrix = scale * rotation * translation;

	if (!m_Animations.empty())
	{
		m_AnimationProgressTime += deltatime;
		m_AnimationProgressTime = (float)fmod(m_AnimationProgressTime, m_Animations[m_CurrentAnimationIndex].m_Duration);
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
		
		if (bone.m_Index ==0 )
		{
			bone.modelMatrix = bone.localMatrix;
		}
		else if (bone.m_ParentIndex != -1 && bone.m_Index != 0)
		{
			bone.modelMatrix = bone.localMatrix * m_Skeleton[bone.m_ParentIndex].modelMatrix;
		}

		//GPU에 전달할 정보
		m_SkeletonPose.modelMatrix[bone.m_Index] = (bone.modelMatrix).Transpose();
	}
}

void SkeletalMesh::Draw(ConstantBuffer* pCBuffer, ID3D11Buffer* pConstantBuffer)
{
	pCBuffer->World = XMMatrixTranspose(m_WorldMatrix);
	m_pDeviceContext->UpdateSubresource(pConstantBuffer, 0, nullptr, pCBuffer, 0, 0);
	for (size_t i = 0; i < m_Meshes.size(); ++i)
	{
		m_Meshes[i].DrawMesh(m_pDeviceContext);
	}
}

void SkeletalMesh::DrawAnimation(ConstantBuffer* pCBuffer,ID3D11Buffer* pConstantBuffer, ID3D11Buffer* pBonePoseBuffer, ID3D11Buffer* pBoneOffsetBuffer)
{
	pCBuffer->IsRigid = m_IsRigid;
	m_WorldMatrix = DirectX::XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) * DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z) * DirectX::XMMatrixTranslation(m_Translation.x, m_Translation.y, m_Translation.z);
	pCBuffer->World = XMMatrixTranspose(m_WorldMatrix);

	for (int i = 0; i < m_Meshes.size(); i++)
	{
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		m_pDeviceContext->IASetVertexBuffers(0, 1, m_Meshes[i].m_pVertexBuffer.GetAddressOf(), &stride, &offset);	// 버텍스 버퍼 입력 조립
		m_pDeviceContext->IASetIndexBuffer(m_Meshes[i].m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		for (UINT j = 0; j < m_Meshes[i].m_Textures.size(); j++)
		{
			Texture t = m_Meshes[i].m_Textures[j];
			m_pDeviceContext->PSSetShaderResources(0, 1, t.m_pTextureSRV.GetAddressOf());
		}

		pCBuffer->World = XMMatrixTranspose(m_WorldMatrix);
		pCBuffer->RefBoneIndex = m_Meshes[i].m_RefBoneIndex;
		 //TODO : UpdateSubresource / VSSetConstantBuffers / pssetconstantbuffer  m_poffsetmatrixbuffer사용
		m_pDeviceContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
		m_pDeviceContext->VSSetConstantBuffers(3, 1, &pBonePoseBuffer);
		if(!m_IsRigid)
		{
			m_pDeviceContext->VSSetConstantBuffers(4, 1, &pBoneOffsetBuffer);
		}

		m_pDeviceContext->UpdateSubresource(pConstantBuffer, 0, nullptr, pCBuffer, 0, 0);
		m_pDeviceContext->UpdateSubresource(pBonePoseBuffer, 0, nullptr, &m_SkeletonPose, 0, 0);
		m_pDeviceContext->UpdateSubresource(pBoneOffsetBuffer, 0, nullptr, &m_SkeletonInfo.m_BoneOffsetMatrices, 0, 0);

		m_pDeviceContext->DrawIndexed(static_cast<UINT>(m_Meshes[i].m_Indices.size()), 0, 0);
	}
}

void SkeletalMesh::CheckAnimationType(const aiScene* pScene)
{
	for (int i = 0; i < pScene->mNumMeshes; ++i)
	{
		if (pScene->mMeshes[i]->mNumBones>0)
		{
			this->m_IsRigid = false;
			return;
		}
	}
}

void SkeletalMesh::Close()
{

}

void SkeletalMesh::ProcessNode(aiNode* node, const aiScene* scene)
{
	if(node!= scene->mRootNode)
	{
		Bone bone;
		bone.m_Name = node->mName.C_Str();
		bone.localMatrix = Matrix{ &node->mTransformation.a1 }.Transpose(); // 노드의 상대적인 행렬
		auto it = m_SkeletonInfo.m_BoneMappingTable.find(bone.m_Name);
		if(it != m_SkeletonInfo.m_BoneMappingTable.end())
		{
			bone.m_Index = it->second;
			if(node->mParent)
			{
				bone.m_ParentIndex = m_SkeletonInfo.GetBoneIndexByName(node->mParent->mName.C_Str());
			}
			
			if(m_HasAnimation)
			{
				for (auto& it : m_Animations[0].m_BoneAnimations)
				{
					if (it.m_Name == bone.m_Name)
					{
						bone.m_pBoneAnimation = &it;
					}
				}
			}
			m_Skeleton.push_back(bone);
		}
	}

	for (UINT j = 0; j < node->mNumMeshes; ++j)
	{
		// 노드의 mMesh는 scene의 mMesh인덱스 번호를 반환
		aiMesh* pMesh = scene->mMeshes[node->mMeshes[j]];
		Mesh mesh = this->ProcessMesh(pMesh, scene);
		mesh.m_RefBoneIndex = m_SkeletonInfo.GetBoneIndexByName(node->mName.C_Str());
		m_Meshes.push_back(mesh);
		m_SkeletonInfo.m_MeshMappingTable.insert({ pMesh->mName.C_Str(), mesh.m_RefBoneIndex });
	}

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		aiNode* pChildNode = node->mChildren[i];
		this->ProcessNode(pChildNode, scene);
	}
}

Mesh SkeletalMesh::ProcessMesh(aiMesh* pMesh, const aiScene* scene)
{
	//if (pMesh->mNumBones > 0) { m_IsRigid = false; }
	Mesh mesh;
	mesh.m_pDevice = m_pDevice;
	std::vector<Vertex> vertices; // 메쉬에 들어갈 정점 정보
	std::vector<UINT> indices; // 인덱스 버퍼에 들어갈 정점의 인덱스
	std::vector<Texture> textures; // 메쉬에 들어갈 텍스쳐들
	for (UINT i = 0; i < pMesh->mNumVertices; ++i) // 버텍스 정보 입력
	{
		Vertex vertex{
			pMesh->mVertices[i].x, 
			pMesh->mVertices[i].y, 
			pMesh->mVertices[i].z,
		};

		if (pMesh->mTextureCoords[0])
		{
			vertex.texture.x = (float)pMesh->mTextureCoords[0][i].x;
			vertex.texture.y = (float)pMesh->mTextureCoords[0][i].y;
		}

		if (pMesh->HasNormals())
		{
			vertex.Normal.x = pMesh->mNormals[i].x;
			vertex.Normal.y = pMesh->mNormals[i].y;
			vertex.Normal.z = pMesh->mNormals[i].z;
		}

		if (pMesh->HasTangentsAndBitangents())
		{
			vertex.Tangent.x = pMesh->mTangents[i].x;
			vertex.Tangent.y = pMesh->mTangents[i].y;
			vertex.Tangent.z = pMesh->mTangents[i].z;

			vertex.BiNormal.x = pMesh->mBitangents[i].x;
			vertex.BiNormal.y = pMesh->mBitangents[i].y;
			vertex.BiNormal.z = pMesh->mBitangents[i].z;
		}
		mesh.m_Vertices.push_back(vertex);
	}

	for (UINT i = 0; i < pMesh->mNumBones; i++)
	{
		aiBone* pBone = pMesh->mBones[i];
		Bone bone;
		bone.m_Name = pBone->mName.C_Str();
		bone.m_Index = m_SkeletonInfo.GetBoneIndexByName(bone.m_Name);
		m_SkeletonInfo.m_BoneOffsetMatrices.modelMatrix[bone.m_Index] = Matrix(&pBone->mOffsetMatrix.a1)/*.Transpose()*/;
		if(!m_IsRigid)
		{
			for (unsigned int j = 0; j < pBone->mNumWeights; j++)
			{
				UINT vertexID = pBone->mWeights[j].mVertexId;
				float vertexWeight = pBone->mWeights[j].mWeight;
				mesh.m_Vertices[vertexID].AddBoneData(bone.m_Index, vertexWeight);
			}
		}
	}

	// 인덱스로 만들어진 폴리곤 면들
	for (UINT i = 0; i < pMesh->mNumFaces; ++i)		// face 정보 입력
	{
		aiFace face = pMesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++) // face가 가지고 있는 인덱스 번호 입력
		{
			mesh.m_Indices.push_back(face.mIndices[j]);
		}
	}

	if (pMesh->mMaterialIndex >= 0) // 머티리얼이 존재한다면
	{
		aiMaterial* pMaterial = scene->mMaterials[pMesh->mMaterialIndex];
		std::string aistr = pMaterial->GetName().C_Str();
		aiString astr;
		aiColor3D diffuseColor(0, 0, 0);

		// 머티리얼 타입별 적용, 실패하면 nullptr 반환
		if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &astr) == AI_SUCCESS)
		{
			for (unsigned int i = 0; i < pMaterial->GetTextureCount(aiTextureType_DIFFUSE); i++)
			{
				std::vector<Texture> Maps = this->LoadMaterialTextures(pMaterial, aiTextureType_DIFFUSE, "texture_diffuse", scene);
				mesh.m_Textures.insert(mesh.m_Textures.end(), Maps.begin(), Maps.end());
			}
		}
		
		if (pMaterial->GetTexture(aiTextureType_AMBIENT, 0, &astr) == AI_SUCCESS)
		{
			for (unsigned int i = 0; i < pMaterial->GetTextureCount(aiTextureType_AMBIENT); i++)
			{
				std::vector<Texture> Maps = this->LoadMaterialTextures(pMaterial, aiTextureType_AMBIENT, "texture_ambient", scene);
				mesh.m_Textures.insert(mesh.m_Textures.end(), Maps.begin(), Maps.end());
			}
		}
		
		if (pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &astr) == AI_SUCCESS)
		{
			for (unsigned int i = 0; i < pMaterial->GetTextureCount(aiTextureType_SPECULAR); i++)
			{
				std::vector<Texture> Maps = this->LoadMaterialTextures(pMaterial, aiTextureType_SPECULAR, "texture_specular", scene);
				mesh.m_Textures.insert(mesh.m_Textures.end(), Maps.begin(), Maps.end());
			}
		}
		
		if (pMaterial->GetTexture(aiTextureType_OPACITY, 0, &astr) == AI_SUCCESS)
		{
			for (unsigned int i = 0; i < pMaterial->GetTextureCount(aiTextureType_OPACITY); i++)
			{
				std::vector<Texture> Maps = this->LoadMaterialTextures(pMaterial, aiTextureType_OPACITY, "texture_opacity", scene);
				mesh.m_Textures.insert(mesh.m_Textures.end(), Maps.begin(), Maps.end());
			}
		}
			
		if (pMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &astr) == AI_SUCCESS)
		{
			for (unsigned int i = 0; i < pMaterial->GetTextureCount(aiTextureType_EMISSIVE); i++)
			{
				std::vector<Texture> Maps = this->LoadMaterialTextures(pMaterial, aiTextureType_EMISSIVE, "texture_emissive", scene);
				mesh.m_Textures.insert(mesh.m_Textures.end(), Maps.begin(), Maps.end());
			}
		}

		//if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) )
		//{
		//	
		//}
	}
	mesh.CreateVertexBuffer(m_pDevice);
	mesh.CreateIndexBuffer(m_pDevice);
	return mesh;
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
		boneanim.m_Name = pNodeAnim->mNodeName.C_Str();
		boneanim.AnimationKeys.resize((size_t)pAnim->mDuration + 1); // 애니메이션 전체 길이의 프레임 수만큼 할당
		for (UINT k = 0; k < pNodeAnim->mNumPositionKeys; k++)
		{
			KeyFrame& keyframe = boneanim.AnimationKeys[(unsigned __int64)pNodeAnim->mPositionKeys[k].mTime];
			keyframe.Position.x = pNodeAnim->mPositionKeys[k].mValue.x;
			keyframe.Position.y = pNodeAnim->mPositionKeys[k].mValue.y;
			keyframe.Position.z = pNodeAnim->mPositionKeys[k].mValue.z;
			keyframe.Time = pNodeAnim->mPositionKeys[k].mTime;
			boneanim.AnimationKeys.push_back(keyframe);
		}			
		
		for (UINT k = 0; k < pNodeAnim->mNumRotationKeys; k++) // 같은 키프레임이라는 전제 하에
		{
			KeyFrame& keyframe = boneanim.AnimationKeys[(unsigned __int64)pNodeAnim->mRotationKeys[k].mTime];
			keyframe.Rotation.x = pNodeAnim->mRotationKeys[k].mValue.x;
			keyframe.Rotation.y = pNodeAnim->mRotationKeys[k].mValue.y;
			keyframe.Rotation.z = pNodeAnim->mRotationKeys[k].mValue.z;
			keyframe.Rotation.w = pNodeAnim->mRotationKeys[k].mValue.w;
			keyframe.Time = pNodeAnim->mRotationKeys[k].mTime;
		}
		
		for (UINT k = 0; k < pNodeAnim->mNumScalingKeys; k++)  // 같은 키프레임이라는 전제 하에
		{
			KeyFrame& keyframe = boneanim.AnimationKeys[(unsigned __int64)pNodeAnim->mScalingKeys[k].mTime];
			keyframe.Scale.x = pNodeAnim->mScalingKeys[k].mValue.x;
			keyframe.Scale.y = pNodeAnim->mScalingKeys[k].mValue.y;
			keyframe.Scale.z = pNodeAnim->mScalingKeys[k].mValue.z;
			keyframe.Time = pNodeAnim->mScalingKeys[k].mTime;
		}
		for_each(boneanim.AnimationKeys.begin(), boneanim.AnimationKeys.end(), [&pAnim](KeyFrame& key) {key.Time /= pAnim->mTicksPerSecond; });
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
					throw std::runtime_error("Texture couldn't be loaded");
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
			throw std::runtime_error("CreatedTextture2D Failed!");
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
