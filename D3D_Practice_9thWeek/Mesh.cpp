#include "Mesh.h"

void SkeletonInfo::CreatefromAiScene(const aiScene* pScene)
{
    aiNode* m_Childnode = pScene->mRootNode->mChildren[0];
    CreateBoneInfo(m_Childnode, pScene);
}

void SkeletonInfo::CreateBoneInfo(const aiNode* pNode, const aiScene* pScene)
{
    BoneInfo boneinfo(pNode);
    boneinfo.Name = pNode->mName.C_Str();
    boneinfo.ParentBoneName = pNode->mParent->mName.C_Str();
    boneinfo.index = Bones.size();
    Bones.push_back(boneinfo);
    m_BoneMappingTable.insert({boneinfo.Name, boneinfo.index});
    for (int i = 0; i < pNode->mNumChildren; i++)
    {
        CreateBoneInfo(pNode->mChildren[i], pScene);
    }
}

BoneInfo* SkeletonInfo::GetBoneInfoByName(const std::string bonename)
{
    for (auto& bone : Bones)
    {
        if (bone.Name == bonename)
        {
			return &bone;
        }
    }
    return nullptr;
}

BoneInfo* SkeletonInfo::GetBoneInfoByIndex(int m_Index)
{
    return &Bones[m_Index];
}

int SkeletonInfo::GetBoneIndexByName(const std::string& name)
{
    return m_BoneMappingTable[name];
}

int SkeletonInfo::GetBoneIndexByMeshName(const std::string& name)
{
    return m_MeshMappingTable[name];
}

const std::string SkeletonInfo::GetBoneName(int m_Index)
{
    std::string name;
    for (auto& m : m_BoneMappingTable)
    {
        if (m.second == m_Index)
            return m.first;
    }
    return std::string();
}

bool SkeletonInfo::IsExist(const std::string& name)
{
    for (auto& it : m_BoneMappingTable)
    {
        if (it.first == name)
            return true;
    }
    return false;
}
