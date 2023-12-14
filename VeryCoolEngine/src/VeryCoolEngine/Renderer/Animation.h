#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Mesh.h"
#include "Bone.h"


struct aiAnimation;
class aiNode;

namespace VeryCoolEngine {
    struct AssimpNodeData
    {
        glm::mat4 transformation;
        std::string name;
        int childrenCount;
        std::vector<AssimpNodeData> children;
    };

    class Animation
    {
    public:
        Animation() = default;

        Animation(const std::string& animationPath, Mesh* model);

        ~Animation()
        {
        }

        Bone* FindBone(const std::string& name)
        {
            auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
                [&](const Bone& Bone)
                {
                    return Bone.GetBoneName() == name;
                }
            );
            if (iter == m_Bones.end()) return nullptr;
            else return &(*iter);
        }


        inline float GetTicksPerSecond() { return m_TicksPerSecond; }

        inline float GetDuration() { return m_Duration; }

        inline const AssimpNodeData& GetRootNode() { return m_RootNode; }

        inline const std::map<std::string, Mesh::BoneInfo>& GetBoneIDMap()
        {
            return m_BoneInfoMap;
        }



        void UpdateAnimation(float dt)
        {
            m_DeltaTime = dt;
            m_CurrentTime += GetTicksPerSecond() * dt;
            m_CurrentTime = fmod(m_CurrentTime, GetDuration());
                if (m_CurrentTime > GetDuration() - 10)
                    m_CurrentTime = 0;
                CalculateBoneTransform(&GetRootNode(), glm::mat4(1.0f));
            
        }


        void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
        {
            std::string nodeName = node->name;
            glm::mat4 nodeTransform = node->transformation;

            Bone* Bone = FindBone(nodeName);

            if (Bone)
            {
                Bone->Update(m_CurrentTime);
                nodeTransform = Bone->GetLocalTransform();
            }

            glm::mat4 globalTransformation = parentTransform * nodeTransform;

            auto boneInfoMap = GetBoneIDMap();
            if (boneInfoMap.find(nodeName) != boneInfoMap.end())
            {
                int index = boneInfoMap[nodeName].id;
                glm::mat4 offset = boneInfoMap[nodeName].offset;
                m_FinalBoneMatrices[index] = globalTransformation * offset;
            }

            for (int i = 0; i < node->childrenCount; i++)
                CalculateBoneTransform(&node->children[i], globalTransformation);
        }

        std::vector<glm::mat4>& GetFinalBoneMatrices()
        {
            return m_FinalBoneMatrices;
        }
    private:
        void ReadMissingBones(const aiAnimation* animation, Mesh& model);

        void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);
        float m_Duration;
        int m_TicksPerSecond;
        std::vector<Bone> m_Bones;
        AssimpNodeData m_RootNode;
        std::map<std::string, Mesh::BoneInfo> m_BoneInfoMap;

        std::vector<glm::mat4> m_FinalBoneMatrices;
        float m_CurrentTime;
        float m_DeltaTime;
    };
}