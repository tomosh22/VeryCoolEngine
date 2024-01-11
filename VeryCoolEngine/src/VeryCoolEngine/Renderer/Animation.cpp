//credit learnopengl.com
#include <vcepch.h>
#include "Animation.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "VeryCoolEngine/AssetHandling/Assets.h"
#include "Mesh.h"
#include "Model.h"

namespace VeryCoolEngine {

    inline glm::vec3 AssimpGLMHelpers::GetGLMVec(const aiVector3D& vec) {
        return glm::vec3(vec.x, vec.y, vec.z);
    }

    inline glm::quat AssimpGLMHelpers::GetGLMQuat(const aiQuaternion& pOrientation)
    {
        return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
    }


    Bone::Bone(const std::string& name, uint32_t uID, const aiNodeAnim* channel)
        :
        m_Name(name),
        m_ID(uID),
        m_LocalTransform(1.0f)
    {
        m_NumPositions = channel->mNumPositionKeys;

        for (uint32_t positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
        {
            aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
            float timeStamp = channel->mPositionKeys[positionIndex].mTime;
            KeyPosition data;
            data.position = AssimpGLMHelpers::GetGLMVec(aiPosition);
            data.timeStamp = timeStamp;
            m_Positions.push_back(data);
        }

        m_NumRotations = channel->mNumRotationKeys;
        for (uint32_t rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
        {
            aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
            float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
            KeyRotation data;
            data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
            data.timeStamp = timeStamp;
            m_Rotations.push_back(data);
        }

        m_NumScalings = channel->mNumScalingKeys;
        for (uint32_t keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
        {
            aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
            float timeStamp = channel->mScalingKeys[keyIndex].mTime;
            KeyScale data;
            data.scale = AssimpGLMHelpers::GetGLMVec(scale);
            data.timeStamp = timeStamp;
            m_Scales.push_back(data);
        }
    }

	Animation::Animation(const std::string& animationPath, VCEModel* model) {
        std::string strPath(MESHDIR);
        strPath += animationPath;
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(strPath, aiProcess_Triangulate);
        assert(scene && scene->mRootNode);
        auto animation = scene->mAnimations[0];
        m_Duration = animation->mDuration;
        m_TicksPerSecond = animation->mTicksPerSecond;
        ReadHeirarchyData(m_RootNode, scene->mRootNode);
        ReadMissingBones(animation, *model);

        m_CurrentTime = 0.0;
        m_FinalBoneMatrices.reserve(1000);

        for (uint32_t i = 0; i < 1000; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
	}

    void Animation::ReadMissingBones(const aiAnimation* animation, VCEModel& model)
    {
        uint32_t size = animation->mNumChannels;

        auto& boneInfoMap = model.m_xBoneInfoMap;
        uint32_t& boneCount = model.m_uBoneCounter;

        for (uint32_t i = 0; i < size; i++)
        {
            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                boneInfoMap[boneName].id = boneCount;
                boneCount++;
            }
            m_Bones.push_back(Bone(channel->mNodeName.data,
                boneInfoMap[channel->mNodeName.data].id, channel));
        }

        m_BoneInfoMap = boneInfoMap;
    }

    void Animation::CalculateBoneTransform(const AssimpNodeData* pxNode, glm::mat4 parentTransform)
    {
        std::string nodeName = pxNode->name;
        glm::mat4 nodeTransform = pxNode->transformation;

        Bone* pxBone = FindBone(nodeName);

        if (pxBone)
        {
            pxBone->Update(m_CurrentTime);
            nodeTransform = pxBone->GetLocalTransform();
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        const std::map<std::string, BoneInfo>& xBoneInfoMap = GetBoneIDMap();
        if (xBoneInfoMap.find(nodeName) != xBoneInfoMap.end())
        {
            uint32_t index = xBoneInfoMap.at(nodeName).id;
            glm::mat4 offset = xBoneInfoMap.at(nodeName).offset;
            m_FinalBoneMatrices[index] = globalTransformation * offset;
        }

        for (uint32_t i = 0; i < pxNode->childrenCount; i++)
            CalculateBoneTransform(&pxNode->children[i], globalTransformation);
    }
    void Animation::ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src)
    {
        dest.name = src->mName.data;
        dest.transformation = Mesh::ConvertMatrixToGLMFormat(src->mTransformation);
        dest.childrenCount = src->mNumChildren;

        for (uint32_t i = 0; i < src->mNumChildren; i++)
        {
            AssimpNodeData newData;
            ReadHeirarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }
}