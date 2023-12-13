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

    private:
        void ReadMissingBones(const aiAnimation* animation, Mesh& model);

        void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);
        float m_Duration;
        int m_TicksPerSecond;
        std::vector<Bone> m_Bones;
        AssimpNodeData m_RootNode;
        std::map<std::string, Mesh::BoneInfo> m_BoneInfoMap;
    };
}