//credit learnopengl.com
#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Mesh.h"
#include "Bone.h"




struct aiAnimation;
class aiNode;

namespace VeryCoolEngine {
    class VCEModel;
    struct BoneInfo;
    struct AssimpNodeData
    {
        glm::mat4 transformation;
        std::string name;
        uint32_t childrenCount;
        std::vector<AssimpNodeData> children;
    };

    class Animation
    {
    public:
        Animation() = default;

        Animation(const std::string& animationPath, VCEModel* model);

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

        inline const std::map<std::string, BoneInfo>& GetBoneIDMap()
        {
            return m_BoneInfoMap;
        }



        void UpdateAnimation(float dt)
        {
            m_DeltaTime = dt;
            m_CurrentTime += GetTicksPerSecond() * dt;
            m_CurrentTime = fmod(m_CurrentTime, GetDuration());
                CalculateBoneTransform(&GetRootNode(), glm::mat4(1.0f));
            
        }


        void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);

        std::vector<glm::mat4>& GetFinalBoneMatrices()
        {
            return m_FinalBoneMatrices;
        }
    private:
        void ReadMissingBones(const aiAnimation* animation, VCEModel& model);

        void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);
        float m_Duration;
        uint32_t m_TicksPerSecond;
        std::vector<Bone> m_Bones;
        AssimpNodeData m_RootNode;
        std::map<std::string, BoneInfo> m_BoneInfoMap;

        std::vector<glm::mat4> m_FinalBoneMatrices;
        float m_CurrentTime;
        float m_DeltaTime;
    };
}