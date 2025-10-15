#include "framework.h"
#include "ModelLoader.h"
#include <directxtk/WICTextureLoader.h>
ModelLoader::ModelLoader() :
	m_pDevice(nullptr), m_pDeviceContext(nullptr), m_meshes(), filepath(),
	textures_loaded(), hwnd(nullptr)
{
}

ModelLoader::~ModelLoader()
{
}

bool ModelLoader::LoadModel(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* devicecontext, std::string _filepath)
{
	//Assimp 임포트 옵션
	Assimp::Importer importer = {};
	unsigned int importFlags = aiProcess_Triangulate | // vertex를 삼각형으로 출력
	aiProcess_GenNormals | // Normal 생성
	aiProcess_GenUVCoords | // UV 좌표 생성
	aiProcess_CalcTangentSpace | // 탄젠트 벡터 생성
	aiProcess_ConvertToLeftHanded | // 왼손 좌표계 
	aiProcess_PreTransformVertices; // 노드의 변환행렬을 미리 적용 -> StaticMesh로 처리할때만 사용
	
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

	processNode(pScene->mRootNode, pScene);
	
	return true;
}

void ModelLoader::Draw(ID3D11DeviceContext* devicecontext)
{
	for (size_t i = 0; i < m_meshes.size(); ++i)
	{
		m_meshes[i].DrawMesh(devicecontext);
	}
}

void ModelLoader::Close()
{

}

void ModelLoader::processNode(aiNode* node, const aiScene* scene)
{
	for (UINT i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* pMesh = scene->mMeshes[node->mMeshes[i]]; // 노드의 mMesh는 scene의 mMesh인덱스 번호를 반환
		m_meshes.push_back(this->processMesh(pMesh, scene));
	}

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		// 트리 구조로 자식들의 노드를 순회하여 mesh 정보 얻어옴
		this->processNode(node->mChildren[i], scene);
	}
}

Mesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene)
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

	for (UINT i = 0; i < mesh->mNumFaces; ++i) // face 정보 입력
	{
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++) // face가 가지고 있는 인덱스 번호 입력
		{
			indices.push_back(face.mIndices[j]);
		}
	}
	
	for (int i = 0; i < mesh->mMaterialIndex+1; i++)
	{
		
	}
	if (mesh->mMaterialIndex >= 0) // 머티리얼이 존재한다면
	{
		aiMaterial* pMaterial = scene->mMaterials[mesh->mMaterialIndex];
		std::string aistr = pMaterial->GetName().C_Str();
		aiString astr;
		//pMaterial->GetTexture()
		// 텍스쳐 맵이 하나씩만 있다고 가정할때, 
		if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &astr) == AI_SUCCESS)
		{
			for (int i = 0; i < pMaterial->GetTextureCount(aiTextureType_DIFFUSE); i++)
			{
				std::vector<Texture> Maps = this->loadMaterialTextures(pMaterial, aiTextureType_DIFFUSE, "texture_diffuse", scene);
				textures.insert(textures.end(), Maps.begin(), Maps.end());
			}
		}
		
		if (pMaterial->GetTexture(aiTextureType_AMBIENT, 0, &astr) == AI_SUCCESS)
		{
			for (int i = 0; i < pMaterial->GetTextureCount(aiTextureType_AMBIENT); i++)
			{
				std::vector<Texture> Maps = this->loadMaterialTextures(pMaterial, aiTextureType_AMBIENT, "texture_ambient", scene);
				textures.insert(textures.end(), Maps.begin(), Maps.end());
			}
		}
		
		if (pMaterial->GetTexture(aiTextureType_SPECULAR, 0, &astr) == AI_SUCCESS)
		{
			for (int i = 0; i < pMaterial->GetTextureCount(aiTextureType_SPECULAR); i++)
			{
				std::vector<Texture> Maps = this->loadMaterialTextures(pMaterial, aiTextureType_SPECULAR, "texture_specular", scene);
				textures.insert(textures.end(), Maps.begin(), Maps.end());
			}
		}
		
		if (pMaterial->GetTexture(aiTextureType_OPACITY, 0, &astr) == AI_SUCCESS)
		{
			for (int i = 0; i < pMaterial->GetTextureCount(aiTextureType_OPACITY); i++)
			{
				std::vector<Texture> Maps = this->loadMaterialTextures(pMaterial, aiTextureType_OPACITY, "texture_opacity", scene);
				textures.insert(textures.end(), Maps.begin(), Maps.end());
			}
		}
		
		if (pMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &astr) == AI_SUCCESS)
		{
			for (int i = 0; i < pMaterial->GetTextureCount(aiTextureType_EMISSIVE); i++)
			{
				std::vector<Texture> Maps = this->loadMaterialTextures(pMaterial, aiTextureType_EMISSIVE, "texture_emissive", scene);
				textures.insert(textures.end(), Maps.begin(), Maps.end());
			}
		}
	}

	return Mesh(m_pDevice, vertices, indices, textures);
}

std::vector<Texture> ModelLoader::loadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName, const aiScene* scene)
{
	std::vector<Texture> textures;
	for (UINT i = 0; i < material->GetTextureCount(type); i++) //텍스쳐 개수만큼 반복
	{
		aiString str;
		material->GetTexture(type, i, &str);
		bool skip = false;
		std::string filename = std::string(str.C_Str());

		for (UINT j = 0; j < textures_loaded.size(); j++)
		{
			if (std::strcmp(textures_loaded[j].path.c_str(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
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
				texture.m_pTextureSRV = loadEmbeddedTexture(embeddedtexture);
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
			this->textures_loaded.push_back(texture);
		}
	}
	return textures;
}

ID3D11ShaderResourceView* ModelLoader::loadEmbeddedTexture(const aiTexture* embeddedTexture)
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
			//MessageBox(hwnd, "CreatedTextture2D Failed!", "Error!", MB_ICONERROR | MB_OK);
		}

		hr = m_pDevice->CreateShaderResourceView(texture2D, nullptr, &texture);
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateShaderResourceView Failed!");
			//MessageBox(hwnd, "CreateShaderResourceView Failed!", "Error!", MB_ICONERROR | MB_OK);
		}

		return texture;
	}

	const size_t size = embeddedTexture->mWidth;

	hr = CreateWICTextureFromMemory(m_pDevice, m_pDeviceContext, reinterpret_cast<const unsigned char*>(embeddedTexture->pcData), size, nullptr, &texture);
	if (FAILED(hr))
	{
		throw std::runtime_error("Texture couldn't be created from memory!");
		//MessageBox(hwnd, "Texture couldn't be created from memory!", "Error!", MB_ICONERROR | MB_OK);
	}

	return texture;
}
