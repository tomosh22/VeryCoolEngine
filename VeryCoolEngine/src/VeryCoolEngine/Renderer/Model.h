//credit learnopengl.com
#pragma once

#include <string>
#include <vector>
#include "Texture.h"
#include "VeryCoolEngine/Transform.h"

#include <glm/glm.hpp>

namespace reactphysics3d {
    class Transform;
    class RigidBody;
}

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;
enum aiTextureType;

namespace VeryCoolEngine {


    class Mesh;
    struct Vertex;

    struct BoneInfo
    {
        uint32_t id;
        glm::mat4 offset;
    };


    class VCEModel
    {
    public:
        std::vector<Texture2D*> m_apxTextures;
        std::vector<Mesh*> m_apxMeshes;
        std::string m_strDirectory;

        glm::mat4 m_xModelMat;

        VCEModel() = default;

        VCEModel(std::string const& path);

        ~VCEModel() {
            delete m_pxAnimation;
            for (Mesh* pxMesh : m_apxMeshes)
                delete pxMesh;
            m_apxMeshes.clear();

            //TODO: is this necessary? these might get cleaned up by the renderer
            for (Texture2D* pxTex : m_apxTextures)
                delete pxTex;
            m_apxTextures.clear();
        }

        void Draw(Shader& shader);

        class Animation* m_pxAnimation = nullptr;

        std::map<std::string, BoneInfo> m_xBoneInfoMap;
        uint32_t m_uBoneCounter = 0;

        void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh);

        bool m_bShowInEditor = true;

        bool m_bUsePhysics = false;
    private:
        
        const aiScene* m_pxScene;


        void ProcessNode(aiNode* node);

        Mesh* ProcessMesh(aiMesh* mesh);

        std::vector<Texture2D*> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

        static void SetVertexBoneData(Vertex& vertex, int boneID, float weight);
    };


}