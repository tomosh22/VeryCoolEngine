//credit learnopengl.com
#pragma once

#include "vcepch.h"

#include "Model.h"

#include "VeryCoolEngine/Application.h"

#include "VeryCoolEngine/AssetHandling/Assets.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

namespace VeryCoolEngine {


    VCEModel::VCEModel(std::string const& path)
    {
        m_strDirectory = path;
        LoadModel(path);
    }

    void VCEModel::Draw(Shader& shader)
    {
        //for (unsigned int i = 0; i < meshes.size(); i++)
            //meshes[i].Draw(shader);
    }

    
    void VCEModel::LoadModel(std::string const& path)
    {
        Assimp::Importer importer;
        m_pxScene = importer.ReadFile(MESHDIR+path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        VCE_ASSERT(!(m_pxScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE), "Assimp scene failed to load");

        ProcessNode(m_pxScene->mRootNode);
    }

    
    void VCEModel::ProcessNode(aiNode* node)
    {
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            meshes.push_back(ProcessMesh(m_pxScene->mMeshes[node->mMeshes[i]]));
        }
        
        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(node->mChildren[i]);
        }

    }

    Mesh* VCEModel::ProcessMesh(aiMesh* mesh)
    {
        Mesh* pxMesh = Mesh::Create();
        
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Texture2D*> textures;

        for (uint32_t i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector;
            
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.pos = vector;
            
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.normal = vector;
            }
            
            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.uv = vec;
                
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.tangent = vector;
                
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.bitangent = vector;
            }
            else
                vertex.uv = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        for (uint32_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        aiMaterial* material = m_pxScene->mMaterials[mesh->mMaterialIndex];
        VCE_TRACE("Model {} material:", m_strDirectory.c_str());
        for (uint32_t i = aiTextureType_NONE + 1; i < AI_TEXTURE_TYPE_MAX; i++)
            VCE_TRACE("Type {}, count {}", i,material->GetTextureCount((aiTextureType)i));
#if 1
        pxMesh->m_pxMaterial = Material::Create();
        if (material->GetTextureCount(aiTextureType_DIFFUSE)){
            VCE_ASSERT(material->GetTextureCount(aiTextureType_DIFFUSE) == 1, "Too many diffuse maps");
            aiString str;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &str);
            const aiTexture* pxTex = m_pxScene->GetEmbeddedTexture(str.C_Str());
            if (pxTex != nullptr) {
                VCE_ASSERT(pxTex->mHeight == 0, "need to add support for non compressed textures");
                Texture2D* pxVceTex = Texture2D::Create();
                pxVceTex->m_pData = malloc(pxTex->mWidth);
                memcpy(pxVceTex->m_pData, pxTex->pcData, pxTex->mWidth);
                pxVceTex->m_uDataLength = pxTex->mWidth;
                VCE_ASSERT(pxVceTex->m_uDataLength > 0, "Incorrect texture data length");
                pxMesh->m_pxMaterial->SetAlbedo(pxVceTex);
            }
        }

        if (material->GetTextureCount(aiTextureType_NORMALS)) {
            VCE_ASSERT(material->GetTextureCount(aiTextureType_NORMALS) == 1, "Too many normal maps");
            aiString str;
            material->GetTexture(aiTextureType_NORMALS, 0, &str);
            const aiTexture* pxTex = m_pxScene->GetEmbeddedTexture(str.C_Str());
            if (pxTex != nullptr) {
                VCE_ASSERT(pxTex->mHeight == 0, "need to add support for non compressed textures");
                Texture2D* pxVceTex = Texture2D::Create();
                pxVceTex->m_pData = malloc(pxTex->mWidth);
                memcpy(pxVceTex->m_pData, pxTex->pcData, pxTex->mWidth);
                pxVceTex->m_uDataLength = pxTex->mWidth;
                VCE_ASSERT(pxVceTex->m_uDataLength > 0, "Incorrect texture data length");
                pxMesh->m_pxMaterial->SetBumpMap(pxVceTex);
            }
        }

        if (material->GetTextureCount(aiTextureType_SHININESS)) {
            VCE_ASSERT(material->GetTextureCount(aiTextureType_SHININESS) == 1, "Too many gloss maps");
            aiString str;
            material->GetTexture(aiTextureType_SHININESS, 0, &str);
            const aiTexture* pxTex = m_pxScene->GetEmbeddedTexture(str.C_Str());
            if (pxTex != nullptr) {
                VCE_ASSERT(pxTex->mHeight == 0, "need to add support for non compressed textures");
                Texture2D* pxVceTex = Texture2D::Create();
                pxVceTex->m_pData = malloc(pxTex->mWidth);
                memcpy(pxVceTex->m_pData, pxTex->pcData, pxTex->mWidth);
                pxVceTex->m_uDataLength = pxTex->mWidth;
                VCE_ASSERT(pxVceTex->m_uDataLength > 0, "Incorrect texture data length");
                //im treating the gloss map as a metallic map which isn't really right
                pxMesh->m_pxMaterial->SetMetallic(pxVceTex);
            }
        }

#endif

        pxMesh->m_axVertices = vertices;
        pxMesh->m_auIndices = indices;
        //pxMesh->m_apxTextures = textures;

        pxMesh->m_uNumVerts = vertices.size();
        pxMesh->m_uNumIndices = indices.size();

        pxMesh->SetShader(Application::GetInstance()->m_pxMeshShader);

        pxMesh->m_uNumBones = mesh->mNumBones;
        pxMesh->m_xBoneMats.resize(mesh->mNumBones);
        for (auto& xMat : pxMesh->m_xBoneMats)
            xMat = glm::identity<glm::mat4>();

        ExtractBoneWeightForVertices(pxMesh->m_axVertices, mesh);

#pragma region copiedfrommeshcpp
        pxMesh->m_pxBufferLayout = new BufferLayout();


        pxMesh->m_uNumIndices = pxMesh->m_auIndices.size();

        pxMesh->m_puIndices = new unsigned int[pxMesh->m_uNumIndices];
        pxMesh->m_pxVertexPositions = new glm::vec3[pxMesh->m_uNumVerts];
        pxMesh->m_pxNormals = new glm::vec3[pxMesh->m_uNumVerts]{ glm::vec3(0,0,0) };
        pxMesh->m_pxTangents = new glm::vec3[pxMesh->m_uNumVerts]{ glm::vec3(0,0,0) };
        pxMesh->m_pxBitangents = new glm::vec3[pxMesh->m_uNumVerts]{ glm::vec3(0,0,0) };
        pxMesh->m_pxUVs = new glm::vec2[pxMesh->m_uNumVerts];
        if (pxMesh->m_uNumBones)
            pxMesh->m_pxBoneDatas = new Mesh::BoneData[pxMesh->m_uNumVerts];


        for (size_t i = 0; i < pxMesh->m_auIndices.size(); i++)pxMesh->m_puIndices[i] = pxMesh->m_auIndices[i];
        for (size_t i = 0; i < pxMesh->m_axVertices.size(); i++) {
            pxMesh->m_pxVertexPositions[i] = pxMesh->m_axVertices[i].pos;
            pxMesh->m_pxUVs[i] = pxMesh->m_axVertices[i].uv;
            pxMesh->m_pxNormals[i] = pxMesh->m_axVertices[i].normal;
            pxMesh->m_pxTangents[i] = pxMesh->m_axVertices[i].tangent;
            pxMesh->m_pxBitangents[i] = pxMesh->m_axVertices[i].bitangent;
            if (pxMesh->m_uNumBones) {
                Mesh::BoneData xData;
                for (uint32_t j = 0; j < MAX_BONES_PER_VERTEX; j++) {
                    xData.m_auIDs[j] = pxMesh->m_axVertices[i].m_BoneIDs[j];
                    xData.m_afWeights[j] = pxMesh->m_axVertices[i].m_Weights[j];
                }
                pxMesh->m_pxBoneDatas[i] = xData;
            }
        }


        int numFloats = 0;
        if (pxMesh->m_pxVertexPositions != nullptr) {
            pxMesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aPosition" });
            numFloats += 3;
        }
        if (pxMesh->m_pxUVs != nullptr) {
            pxMesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float2, "_aUV" });
            numFloats += 2;
        }
        if (pxMesh->m_pxNormals != nullptr) {
            pxMesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aNormal" });
            numFloats += 3;
        }
        if (pxMesh->m_pxTangents != nullptr) {
            pxMesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aTangent" });
            numFloats += 3;
        }
        if (pxMesh->m_pxBitangents != nullptr) {
            pxMesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aBitangent" });
            numFloats += 3;
        }
        if (pxMesh->m_pxBoneDatas != nullptr) {
            pxMesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::UInt4, "_aBoneIndices0to4" });
            pxMesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::UInt4, "_aBoneIndices5to8" });
            pxMesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float4, "_aBoneWeights0to4" });
            pxMesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float4, "_aBoneWeights5to8" });
            numFloats += 16;
        }

        pxMesh->m_pVerts = new float[pxMesh->m_uNumVerts * numFloats];

        size_t index = 0;
        for (uint32_t i = 0; i < pxMesh->m_uNumVerts; i++)
        {
            if (pxMesh->m_pxVertexPositions != nullptr) {
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxVertexPositions[i].x;
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxVertexPositions[i].y;
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxVertexPositions[i].z;
            }

            if (pxMesh->m_pxUVs != nullptr) {
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxUVs[i].x;
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxUVs[i].y;
            }
            if (pxMesh->m_pxNormals != nullptr) {
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxNormals[i].x;
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxNormals[i].y;
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxNormals[i].z;
            }
            if (pxMesh->m_pxTangents != nullptr) {
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxTangents[i].x;
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxTangents[i].y;
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxTangents[i].z;
            }
            if (pxMesh->m_pxBitangents != nullptr) {
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBitangents[i].x;
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBitangents[i].y;
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBitangents[i].z;
            }
            if (pxMesh->m_pxBoneDatas != nullptr) {
                ((uint32_t*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_auIDs[0];
                ((uint32_t*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_auIDs[1];
                ((uint32_t*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_auIDs[2];
                ((uint32_t*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_auIDs[3];
                ((uint32_t*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_auIDs[4];
                ((uint32_t*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_auIDs[5];
                ((uint32_t*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_auIDs[6];
                ((uint32_t*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_auIDs[7];

                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_afWeights[0];
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_afWeights[1];
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_afWeights[2];
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_afWeights[3];
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_afWeights[4];
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_afWeights[5];
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_afWeights[6];
                ((float*)pxMesh->m_pVerts)[index++] = pxMesh->m_pxBoneDatas[i].m_afWeights[7];
            }
        }

        pxMesh->m_pxBufferLayout->CalculateOffsetsAndStrides();
#pragma endregion




        return pxMesh;
    }

    std::vector<Texture2D*> VCEModel::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
    {
        std::vector<Texture2D*> textures;
        for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            bool skip = false;
            for (uint32_t j = 0; j < m_apxTextures.size(); j++)
            {
                if (std::strcmp(m_apxTextures[j]->_filePath.data(), str.C_Str()) == 0)
                {
                    textures.push_back(m_apxTextures[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
                Texture2D* texture = Texture2D::Create(str.C_Str(), false);
                texture->_filePath = str.C_Str();
                textures.push_back(texture);
                m_apxTextures.push_back(texture);
            }
        }
        return textures;
    }

    void VCEModel::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh)
    {
        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            uint32_t boneID;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if (m_xBoneInfoMap.find(boneName) == m_xBoneInfoMap.end())
            {
                m_xBoneInfoMap[boneName] = {
                    m_uBoneCounter,
                    Mesh::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix)
                };
                boneID = m_uBoneCounter++;
            }
            else
            {
                boneID = m_xBoneInfoMap[boneName].id;
            }
            aiVertexWeight* weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                Mesh::SetVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
    }

    void VCEModel::SetVertexBoneData(Vertex& vertex, int boneID, float weight)
    {
        vertex.m_BoneIDs[vertex.m_uNumBones] = boneID;
        vertex.m_Weights[vertex.m_uNumBones] = weight;
        vertex.m_uNumBones++;
        return;
    }

}