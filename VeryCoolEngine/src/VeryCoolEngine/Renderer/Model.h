//credit learnopengl.com
#pragma once

#include <string>
#include <vector>
#include "Texture.h"
#include "VeryCoolEngine/Transform.h"

#include <glm/glm.hpp>

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
        std::vector<Mesh*> meshes;
        std::string m_strDirectory;

        Transform m_xTransform;

        VCEModel(std::string const& path);

        void Draw(Shader& shader);

        

        std::map<std::string, BoneInfo> m_xBoneInfoMap;
        uint32_t m_uBoneCounter = 0;

        void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh);
    private:
        
        const aiScene* m_pxScene;

        void LoadModel(std::string const& path);

        void ProcessNode(aiNode* node);

        Mesh* ProcessMesh(aiMesh* mesh);

        std::vector<Texture2D*> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

        static void SetVertexBoneData(Vertex& vertex, int boneID, float weight);
    };


}