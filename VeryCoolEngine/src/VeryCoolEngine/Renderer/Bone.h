//credit learnopengl.com
#pragma once
#include <vcepch.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

typedef float ai_real;
template<typename TReal>
class aiMatrix4x4t;
typedef aiMatrix4x4t<ai_real> aiMatrix4x4;

template <typename TReal>
class aiVector3t;
typedef aiVector3t<ai_real> aiVector3D;

template <typename TReal>
class aiQuaterniont;
using aiQuaternion = aiQuaterniont<ai_real>;

struct aiNodeAnim;
namespace VeryCoolEngine {
    class AssimpGLMHelpers
    {
    public:


        static inline glm::vec3 GetGLMVec(const aiVector3D& vec);

        static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation);
    };

    struct KeyPosition
    {
        glm::vec3 position;
        float timeStamp;
    };

    struct KeyRotation
    {
        glm::quat orientation;
        float timeStamp;
    };

    struct KeyScale
    {
        glm::vec3 scale;
        float timeStamp;
    };

    class Bone
    {
    private:
        std::vector<KeyPosition> m_Positions;
        std::vector<KeyRotation> m_Rotations;
        std::vector<KeyScale> m_Scales;
        uint32_t m_NumPositions;
        uint32_t m_NumRotations;
        uint32_t m_NumScalings;

        glm::mat4 m_LocalTransform;
        std::string m_Name;
        uint32_t m_ID;

    public:

        Bone(const std::string& name, uint32_t uID, const aiNodeAnim* channel);

        void Update(float animationTime)
        {
            m_LocalTransform = InterpolatePosition(animationTime) * InterpolateRotation(animationTime) * InterpolateScaling(animationTime);
        }

        glm::mat4 GetLocalTransform() { return m_LocalTransform; }
        std::string GetBoneName() const { return m_Name; }
        uint32_t GetBoneID() { return m_ID; }

        uint32_t GetPositionIndex(float animationTime)
        {
            for (uint32_t index = 0; index < m_NumPositions - 1; ++index)
            {
                if (animationTime < m_Positions[index + 1].timeStamp)
                    return index;
            }
            return 0;
        }

        uint32_t GetRotationIndex(float animationTime)
        {
            for (uint32_t index = 0; index < m_NumRotations - 1; ++index)
            {
                if (animationTime < m_Rotations[index + 1].timeStamp)
                    return index;
            }
            return 0;
        }

        uint32_t GetScaleIndex(float animationTime)
        {
            for (uint32_t index = 0; index < m_NumScalings - 1; ++index)
            {
                if (animationTime < m_Scales[index + 1].timeStamp)
                    return index;
            }
            return 0;
        }

    private:

        float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
        {
            float scaleFactor = 0.0f;
            float midWayLength = animationTime - lastTimeStamp;
            float framesDiff = nextTimeStamp - lastTimeStamp;
            scaleFactor = midWayLength / framesDiff;
            return scaleFactor;
        }

        glm::mat4 InterpolatePosition(float animationTime)
        {
            if (m_NumPositions == 1)
                return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

            uint32_t p0Index = GetPositionIndex(animationTime);
            uint32_t p1Index = p0Index + 1;
            float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
                m_Positions[p1Index].timeStamp, animationTime);
            glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position,
                m_Positions[p1Index].position, scaleFactor);
            return glm::translate(glm::mat4(1.0f), finalPosition);
        }

        glm::mat4 InterpolateRotation(float animationTime)
        {
            if (m_NumRotations == 1)
            {
                auto rotation = glm::normalize(m_Rotations[0].orientation);
                return glm::toMat4(rotation);
            }

            uint32_t p0Index = GetRotationIndex(animationTime);
            uint32_t p1Index = p0Index + 1;
            float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp,
                m_Rotations[p1Index].timeStamp, animationTime);
            glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation,
                m_Rotations[p1Index].orientation, scaleFactor);
            finalRotation = glm::normalize(finalRotation);
            return glm::toMat4(finalRotation);
        }

        /*figures out which scaling keys to interpolate b/w and performs the interpolation
        and returns the scale matrix*/
        glm::mat4 InterpolateScaling(float animationTime)
        {
            if (m_NumScalings == 1)
                return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

            uint32_t p0Index = GetScaleIndex(animationTime);
            uint32_t p1Index = p0Index + 1;
            float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
                m_Scales[p1Index].timeStamp, animationTime);
            glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale
                , scaleFactor);
            return glm::scale(glm::mat4(1.0f), finalScale);
        }

    };
}