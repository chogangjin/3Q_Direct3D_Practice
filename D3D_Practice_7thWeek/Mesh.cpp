#include "Mesh.h"

void SkeletonInfo::CreatefromAiScene(const aiScene* pScene)
{
    aiNode* m_Childnode = pScene->mRootNode->mChildren[0];
    CreateBoneInfo(m_Childnode, pScene);
}

void SkeletonInfo::CreateBoneInfo(const aiNode* pNode, const aiScene* pScene)
{
    BoneInfo boneinfo(pNode);
    boneinfo.ParentBoneName = pNode->mParent->mName.C_Str();
    Bones.push_back(boneinfo);
    for (int i = 0; i < pNode->mNumChildren; i++)
    {
        CreateBoneInfo(pNode->mChildren[i], pScene);
    }
}

BoneInfo* SkeletonInfo::GetBoneInfoByName(const std::string)
{

    return nullptr;
}

BoneInfo* SkeletonInfo::GetBoneInfoByIndex(int index)
{
    return nullptr;
}

int SkeletonInfo::GetBoneIndexByName(const std::string& name)
{
    return m_BoneMappingTable[name];
}

int SkeletonInfo::GetBoneIndexByMeshName(const std::string& name)
{
    return m_MeshMappingTable[name];
}

const std::string SkeletonInfo::GetBoneName(int index)
{
    std::string name;
    for (auto& m : m_BoneMappingTable)
    {
        if (m.second == index)
            return m.first;
    }
    return std::string();
}
