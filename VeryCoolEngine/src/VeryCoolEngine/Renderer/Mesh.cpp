//credit learnopengl.com
#include "vcepch.h"
#include "Mesh.h"
#include "Platform/Vulkan/VulkanMesh.h"

#include <unordered_set>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "VeryCoolEngine/AssetHandling/Assets.h"

#include "Bone.h"

namespace VeryCoolEngine {

	Mesh* Mesh::Create() {
#ifdef VCE_OPENGL
		return new OpenGLMesh();
#endif
#ifdef VCE_VULKAN
		return new VulkanMesh();
#endif
	}
	//credit rich davison
	//#todo cleanup, was written on laptop
	Mesh* Mesh::GenerateGenericHeightmap(uint32_t width, uint32_t height)
	{
		Mesh* mesh = Mesh::Create();
		mesh->m_pxBufferLayout = new BufferLayout();
		glm::vec3 vertexScale = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec2 textureScale = glm::vec2(100,100);
		mesh->m_uNumVerts = width * height;
		mesh->m_uNumIndices = (width - 1) * (height - 1) * 6;
		mesh->m_pxVertexPositions = new glm::vec3[mesh->m_uNumVerts];
		mesh->m_pxUVs = new glm::vec2[mesh->m_uNumVerts];
		mesh->m_pxNormals = new glm::vec3[mesh->m_uNumVerts];
		mesh->m_pxTangents = new glm::vec3[mesh->m_uNumVerts];
		for (size_t i = 0; i < mesh->m_uNumVerts; i++)
		{
			mesh->m_pxNormals[i] = { 0,0,0 };
			mesh->m_pxTangents[i] = { 0,0,0 };
		}
		mesh->m_puIndices = new unsigned int[mesh->m_uNumIndices];

		

		for (int z = 0; z < height; ++z) {
			for (int x = 0; x < width; ++x) {
				int offset = (z * width) + x;
				mesh->m_pxVertexPositions[offset] = glm::vec3(x, 100 + rand() % 1/*#todo read height tex*/, z) * vertexScale;
				glm::vec2 fUV = glm::vec2(x, z) / textureScale;
				mesh->m_pxUVs[offset] = fUV;
			}
		}

		size_t i = 0;
		for (int z = 0; z < height - 1; ++z) {
			for (int x = 0; x < width - 1; ++x) {
				int a = (z * width) + x;
				int b = (z * width) + x + 1;
				int c = ((z + 1) * width) + x + 1;
				int d = ((z + 1) * width) + x;
				mesh->m_puIndices[i++] = a;
				mesh->m_puIndices[i++] = c;
				mesh->m_puIndices[i++] = b;
				mesh->m_puIndices[i++] = c;
				mesh->m_puIndices[i++] = a;
				mesh->m_puIndices[i++] = d;
			}
		}
		
		mesh->GenerateNormals();
		mesh->GenerateTangents();

		int numFloats = 0;
		if (mesh->m_pxVertexPositions != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aPosition" });
			numFloats += 3;
		}
		if (mesh->m_pxUVs != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float2, "_aUV" });
			numFloats += 2;
		}
		if (mesh->m_pxNormals != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aNormal" });
			numFloats += 3;
		}
		if (mesh->m_pxTangents != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aTangent" });
			numFloats += 3;
		}

		mesh->m_pVerts = new float[mesh->m_uNumVerts * numFloats];

		size_t index = 0;
		for (int i = 0; i < mesh->m_uNumVerts; i++)
		{
			if (mesh->m_pxVertexPositions != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].z;
			}

			if (mesh->m_pxUVs != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].y;
			}
			if (mesh->m_pxNormals != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].z;
			}
			if (mesh->m_pxTangents != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].z;
			}
		}

		mesh->m_pxBufferLayout->CalculateOffsetsAndStrides();

		return mesh;
	}
	
	Mesh* Mesh::GenerateQuad(float fScale /*= 1.0f*/) {
		Mesh* mesh = Mesh::Create();
		mesh->m_pxBufferLayout = new BufferLayout();
		mesh->m_uNumVerts = 4;
		mesh->m_uNumIndices = 6;
		mesh->m_pxVertexPositions = new glm::vec3[mesh->m_uNumVerts];
		mesh->m_pxUVs = new glm::vec2[mesh->m_uNumVerts];

		mesh->m_puIndices = new unsigned int[mesh->m_uNumIndices] {0, 2, 1, 2, 3, 1};

		mesh->m_pxVertexPositions[0] = { 0.5,0.5,1 };
		mesh->m_pxVertexPositions[1] = { 0.5,-0.5,1 };
		mesh->m_pxVertexPositions[2] = { -0.5,0.5,1 };
		mesh->m_pxVertexPositions[3] = { -0.5,-0.5,1 };

		mesh->m_pxVertexPositions[0] *= fScale;
		mesh->m_pxVertexPositions[1] *= fScale;
		mesh->m_pxVertexPositions[2] *= fScale;
		mesh->m_pxVertexPositions[3] *= fScale;

		mesh->m_pxUVs[0] = { 1,0 };
		mesh->m_pxUVs[1] = { 1,1 };
		mesh->m_pxUVs[2] = { 0,0 };
		mesh->m_pxUVs[3] = { 0,1 };

		int numFloats = 0;
		if (mesh->m_pxVertexPositions != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aPosition" });
			numFloats += 3;
		}
		if (mesh->m_pxUVs != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float2, "_aUV" });
			numFloats += 2;
		}
		if (mesh->m_pxNormals != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aNormal" });
			numFloats += 3;
		}
		if (mesh->m_pxTangents != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aTangent" });
			numFloats += 3;
		}

		mesh->m_pVerts = new float[mesh->m_uNumVerts * numFloats];

		size_t index = 0;
		for (int i = 0; i < mesh->m_uNumVerts; i++)
		{
			if (mesh->m_pxVertexPositions != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].z;
			}

			if (mesh->m_pxUVs != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].y;
			}
			if (mesh->m_pxNormals != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].z;
			}
			if (mesh->m_pxTangents != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].z;
			}
		}

		mesh->m_pxBufferLayout->CalculateOffsetsAndStrides();

		return mesh;
	}


	Mesh* Mesh::GenerateVulkanTest()
	{
		Mesh* mesh = Mesh::Create();
		mesh->m_pxBufferLayout = new BufferLayout();
		mesh->m_uNumVerts = 4;
		mesh->m_uNumIndices = 6;
		mesh->m_pxVertexPositions = new glm::vec3[mesh->m_uNumVerts];
		mesh->m_pxNormals = new glm::vec3[mesh->m_uNumVerts];

		mesh->m_puIndices = new unsigned int[mesh->m_uNumIndices] {0, 2, 1,1,2,3};

		/*mesh->vertexPositions[0] = { -0.5,-0.5,1 };
		mesh->vertexPositions[1] = { 0.5,-0.5,1 };
		mesh->vertexPositions[2] = { -0.5,0.5,1 };
		mesh->vertexPositions[3] = { 0.5,0.5,1 };*/

		mesh->m_pxVertexPositions[0] = { -50,-50,1 };
		mesh->m_pxVertexPositions[1] = { 50,-50,1 };
		mesh->m_pxVertexPositions[2] = { -50,50,1 };
		mesh->m_pxVertexPositions[3] = { 50,50,1 };

		mesh->m_pxNormals[0] = { 1,0,0 };
		mesh->m_pxNormals[1] = { 0,1,0 };
		mesh->m_pxNormals[2] = { 0,0,1 };
		mesh->m_pxNormals[3] = { 1,1,1 };

#pragma region MoveToGenericInitFunction
		int numFloats = 0;
		if (mesh->m_pxVertexPositions != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aPosition" });
			numFloats += 3;
		}
		if (mesh->m_pxUVs != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float2, "_aUV" });
			numFloats += 2;
		}
		if (mesh->m_pxNormals != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aNormal" });
			numFloats += 3;
		}
		if (mesh->m_pxTangents != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aTangent" });
			numFloats += 3;
		}

		mesh->m_pVerts = new float[mesh->m_uNumVerts * numFloats];

		size_t index = 0;
		for (int i = 0; i < mesh->m_uNumVerts; i++)
		{
			if (mesh->m_pxVertexPositions != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].z;
			}

			if (mesh->m_pxUVs != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].y;
			}
			if (mesh->m_pxNormals != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].z;
			}
			if (mesh->m_pxTangents != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].z;
			}
		}

		mesh->m_pxBufferLayout->CalculateOffsetsAndStrides();
#pragma endregion
		return mesh;
	}
	

	//struct CompareVec3 final
	//{
	//	size_t operator()(const glm::vec3& k)const
	//	{
	//		return std::hash<int>()(k.x) ^ std::hash<int>()(k.y) ^ std::hash<int>()(k.z);
	//	}
	//
	//	bool operator()(const glm::vec3& a, const glm::vec3& b)const
	//	{
	//		return a.x == b.x && a.y == b.y && a.z == b.z;
	//	}
	//};

	

	


	
	
#if 0
	void ReadNodeHierarchy(Mesh* pxVCEMesh, aiMesh* pxAiMesh, const aiScene* pxScene, aiNode* pNode, glm::mat4& ParentTransform)
	{
		std::string NodeName(pNode->mName.data);

		const aiAnimation* pAnimation = pxScene->mAnimations[0];

		glm::mat4 NodeTransformation = *reinterpret_cast<glm::mat4*>(&pNode->mTransformation);

		auto pNodeAnimIt = pxVCEMesh->m_xNodeNameToAnim.find(NodeName);

		if (pNodeAnimIt != pxVCEMesh->m_xNodeNameToAnim.end()) {
			aiNodeAnim* pNodeAnim = pNodeAnimIt->second;
			aiVector3D Scaling = pNodeAnim->mScalingKeys[0].mValue;
			glm::vec3 xScale = *reinterpret_cast<glm::vec3*>(&Scaling);
			glm::mat4 xScaleMat = glm::scale(glm::identity<glm::mat4>(), xScale);
			

			aiQuaternion RotationQ = pNodeAnim->mRotationKeys[0].mValue;
			glm::quat xRot = *reinterpret_cast<glm::quat*>(&RotationQ);
			glm::mat4 xRotMat = Transform::RotationMatFromQuat(xRot);

			aiVector3D Translation = pNodeAnim->mPositionKeys[0].mValue;
			glm::vec3 xTrans = *reinterpret_cast<glm::vec3*>(&Translation);
			glm::mat4 xTransMat = glm::translate(glm::identity<glm::mat4>(), xTrans);

			NodeTransformation = xTransMat * xRotMat * xScaleMat;
		}

		glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

		if (pxVCEMesh->m_xBoneNameToIndex.find(NodeName) != pxVCEMesh->m_xBoneNameToIndex.end()) {
			uint32_t BoneIndex = pxVCEMesh->m_xBoneNameToIndex[NodeName];
			pxVCEMesh->m_xBoneMats.at(BoneIndex) = GlobalTransformation * *reinterpret_cast<glm::mat4*>(&pxAiMesh->mBones[BoneIndex]->mOffsetMatrix);
				//m_BoneInfo[BoneIndex].BoneOffset;
		}

		for (uint32_t i = 0; i < pNode->mNumChildren; i++) {
			ReadNodeHierarchy(pxVCEMesh, pxAiMesh, pxScene, pNode->mChildren[i], GlobalTransformation);
		}
	}
#endif

	glm::mat4 Mesh::ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
	{
		glm::mat4 to;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
		to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
		to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
		to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
		return to;
	}

	void Mesh::ExtractBoneWeightForVertices(Mesh* pxMesh, std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
	{
#if 0
		for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			int boneID = -1;
			std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
			if (pxMesh->m_BoneInfoMap.find(boneName) == pxMesh->m_BoneInfoMap.end())
			{
				Mesh::BoneInfo newBoneInfo;
				newBoneInfo.id = pxMesh->m_BoneCounter;
				newBoneInfo.offset = Mesh::ConvertMatrixToGLMFormat(
					mesh->mBones[boneIndex]->mOffsetMatrix);
				pxMesh->m_BoneInfoMap[boneName] = newBoneInfo;
				boneID = pxMesh->m_BoneCounter;
				pxMesh->m_BoneCounter++;
			}
			else
			{
				boneID = pxMesh->m_BoneInfoMap[boneName].id;
			}
			assert(boneID != -1);
			aiVertexWeight* weights = mesh->mBones[boneIndex]->mWeights;
			int numWeights = mesh->mBones[boneIndex]->mNumWeights;

			for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
			{
				int vertexId = weights[weightIndex].mVertexId;
				float weight = weights[weightIndex].mWeight;
				assert(vertexId <= pxMesh->m_uNumVerts);
				Mesh::SetVertexBoneData(vertices[vertexId], boneID, weight);
			}
		}
#endif
	}

	

	Mesh* Mesh::FromFile(const std::string& path, bool swapYZ /* = false*/)
	{
		Mesh* mesh = Mesh::Create();

		std::string strPath(MESHDIR);
		strPath += path;

		Assimp::Importer importer;
		const aiScene* pxScene = importer.ReadFile(strPath.c_str(),
			aiProcess_CalcTangentSpace |
			aiProcess_GenSmoothNormals |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);

		VCE_ASSERT(pxScene != nullptr, "null scene");

#if 0
		if (pxScene->mNumAnimations) {
			std::vector<aiMesh*> xMeshes;
			for (uint32_t i = 0; i < pxScene->mNumMeshes; i++) {
				xMeshes.push_back(pxScene->mMeshes[i]);
			}
			aiMesh* pxMesh = xMeshes.at(uMeshIndex);
			aiAnimation* pxAnim = pxScene->mAnimations[0];

			mesh->m_uNumBones = pxMesh->mNumBones;

			//fill map with empty vector for each vertex
			for (uint32_t i = 0; i < pxMesh->mNumVertices; i++) {
				mesh->m_xBoneInfluences.insert({ i, {}});
			}

			for (uint32_t uBoneID = 0; uBoneID < pxMesh->mNumBones; uBoneID++) {
				int boneID = -1;
				std::string boneName = pxMesh->mBones[uBoneID]->mName.C_Str();
				if (mesh->m_BoneInfoMap.find(boneName) == mesh->m_BoneInfoMap.end())
				{
					BoneInfo newBoneInfo;
					newBoneInfo.id = mesh->m_BoneCounter;
					newBoneInfo.offset = ConvertMatrixToGLMFormat(
						pxMesh->mBones[uBoneID]->mOffsetMatrix);
					mesh->m_BoneInfoMap[boneName] = newBoneInfo;
					boneID = mesh->m_BoneCounter;
					mesh->m_BoneCounter++;
				}
				else
				{
					boneID = mesh->m_BoneInfoMap[boneName].id;
				}
				assert(boneID != -1);
				auto weights = pxMesh->mBones[uBoneID]->mWeights;
				int numWeights = pxMesh->mBones[uBoneID]->mNumWeights;

				for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
				{
					int vertexId = weights[weightIndex].mVertexId;
					float weight = weights[weightIndex].mWeight;
					VCE_ASSERT(vertexId <= pxMesh->mNumVertices, "VertexID too big");
					SetVertexBoneData(vertices[vertexId], boneID, weight);
				}
			}
			for (uint32_t i = 0; i < pxMesh->mNumVertices; i++) {
				float fTotalWeight = 0;
				for (auto& [uBoneID, fWeight] : mesh->m_xBoneInfluences.at(i))
					fTotalWeight += fWeight;
				VCE_ASSERT(fTotalWeight > 0.9f && fTotalWeight < 1.1f, "dodgy weights");
				BoneData xBoneData;
				for (uint32_t j = 0; j < mesh->m_xBoneInfluences.at(i).size(); j++) {
					xBoneData.m_auIDs[j] = mesh->m_xBoneInfluences.at(i).at(j).first;
					xBoneData.m_afWeights[j] = mesh->m_xBoneInfluences.at(i).at(j).second;
				}
				mesh->m_xBoneData.push_back(xBoneData);
			}


			for (uint32_t i = 0; i < pxAnim->mNumChannels; i++) {
				aiNodeAnim* pxNodeAnim = pxAnim->mChannels[i];
				mesh->m_xNodeNameToAnim.insert({ std::string(pxNodeAnim->mNodeName.C_Str()),pxNodeAnim });
				bool a = false;
			}
			glm::mat4 identity = glm::identity<glm::mat4>();
			ReadNodeHierarchy(mesh, pxMesh, pxScene,pxScene->mRootNode, identity);
			bool a = false;
		}
		
#endif

		
		//#TO_TODO: do i care about any other meshes?
		aiMesh* pxMesh = pxScene->mMeshes[0];

		mesh->m_uNumBones = pxMesh->mNumBones;

		mesh->m_uNumVerts = pxMesh->mNumVertices;

		mesh->m_xBoneMats.resize(pxMesh->mNumBones);
		for (auto& xMat : mesh->m_xBoneMats)
			xMat = glm::identity<glm::mat4>();


		for (uint32_t i = 0; i < pxMesh->mNumVertices; i++) {
			const aiVector3D* pxPos = &(pxMesh->mVertices[i]);
			const aiVector3D* pxNormal = &(pxMesh->mNormals[i]);
			const aiVector3D* pxTexCoord = &(pxMesh->mTextureCoords[0][i]);
			const aiVector3D* pxTangent = &(pxMesh->mTangents[i]);
			const aiVector3D* pxBitangent = &(pxMesh->mBitangents[i]);

			Vertex v;
			v.pos = glm::vec3(pxPos->x, pxPos->y, pxPos->z);
			v.uv = glm::vec2(pxTexCoord->x, pxTexCoord->y);
			v.normal = glm::vec3(pxNormal->x, pxNormal->y, pxNormal->z);
			v.tangent = glm::vec3(pxTangent->x, pxTangent->y, pxTangent->z);
			v.bitangent = glm::vec3(pxBitangent->x, pxBitangent->y, pxBitangent->z);
			//if (mesh->m_uNumBones)
				//v.m_xBoneData = mesh->m_xBoneData[i];
			mesh->m_axVertices.push_back(v);
		}
		ExtractBoneWeightForVertices(mesh, mesh->m_axVertices, pxMesh, pxScene);

		for (uint32_t i = 0; i < pxMesh->mNumFaces; i++) {
			VCE_ASSERT(pxMesh->mFaces[i].mNumIndices == 3, "Face isn't a triangle");
			mesh->m_auIndices.push_back(pxMesh->mFaces[i].mIndices[0]);
			mesh->m_auIndices.push_back(pxMesh->mFaces[i].mIndices[1]);
			mesh->m_auIndices.push_back(pxMesh->mFaces[i].mIndices[2]);
		}


		
		mesh->m_pxBufferLayout = new BufferLayout();

		
		mesh->m_uNumIndices = mesh->m_auIndices.size();

		mesh->m_puIndices = new unsigned int[mesh->m_uNumIndices];
		mesh->m_pxVertexPositions = new glm::vec3[mesh->m_uNumVerts];
		mesh->m_pxNormals = new glm::vec3[mesh->m_uNumVerts]{ glm::vec3(0,0,0) };
		mesh->m_pxTangents = new glm::vec3[mesh->m_uNumVerts]{ glm::vec3(0,0,0) };
		mesh->m_pxBitangents = new glm::vec3[mesh->m_uNumVerts]{ glm::vec3(0,0,0) };
		mesh->m_pxUVs = new glm::vec2[mesh->m_uNumVerts];
		if(mesh->m_uNumBones)
			mesh->m_pxBoneDatas = new BoneData[mesh->m_uNumVerts];


		for (size_t i = 0; i < mesh->m_auIndices.size(); i++)mesh->m_puIndices[i] = mesh->m_auIndices[i];
		for (size_t i = 0; i < mesh->m_axVertices.size(); i++) {
			mesh->m_pxVertexPositions[i] = mesh->m_axVertices[i].pos;
			mesh->m_pxUVs[i] = mesh->m_axVertices[i].uv;
			mesh->m_pxNormals[i] = mesh->m_axVertices[i].normal;
			mesh->m_pxTangents[i] = mesh->m_axVertices[i].tangent;
			mesh->m_pxBitangents[i] = mesh->m_axVertices[i].bitangent;
			if (mesh->m_uNumBones) {
				BoneData xData;
				for (uint32_t j = 0; j < MAX_BONES_PER_VERTEX; j++) {
					xData.m_auIDs[j] = mesh->m_axVertices[i].m_BoneIDs[j];
					xData.m_afWeights[j] = mesh->m_axVertices[i].m_Weights[j];
				}
				mesh->m_pxBoneDatas[i] = xData;
			}
		}


		int numFloats = 0;
		if (mesh->m_pxVertexPositions != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aPosition" });
			numFloats += 3;
		}
		if (mesh->m_pxUVs != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float2, "_aUV" });
			numFloats += 2;
		}
		if (mesh->m_pxNormals != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aNormal" });
			numFloats += 3;
		}
		if (mesh->m_pxTangents != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aTangent" });
			numFloats += 3;
		}
		if (mesh->m_pxBitangents != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float3, "_aBitangent" });
			numFloats += 3;
		}
		if (mesh->m_pxBoneDatas != nullptr) {
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::UInt4, "_aBoneIndices0to4" });
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::UInt4, "_aBoneIndices5to8" });
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float4, "_aBoneWeights0to4" });
			mesh->m_pxBufferLayout->GetElements().push_back({ ShaderDataType::Float4, "_aBoneWeights5to8" });
			numFloats += 16;
		}

		mesh->m_pVerts = new float[mesh->m_uNumVerts * numFloats];

		size_t index = 0;
		for (int i = 0; i < mesh->m_uNumVerts; i++)
		{
			if (mesh->m_pxVertexPositions != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxVertexPositions[i].z;
			}

			if (mesh->m_pxUVs != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxUVs[i].y;
			}
			if (mesh->m_pxNormals != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxNormals[i].z;
			}
			if (mesh->m_pxTangents != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxTangents[i].z;
			}
			if (mesh->m_pxBitangents != nullptr) {
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxBitangents[i].x;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxBitangents[i].y;
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxBitangents[i].z;
			}
			if (mesh->m_pxBoneDatas != nullptr) {
				((uint32_t*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_auIDs[0];
				((uint32_t*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_auIDs[1];
				((uint32_t*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_auIDs[2];
				((uint32_t*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_auIDs[3];
				((uint32_t*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_auIDs[4];
				((uint32_t*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_auIDs[5];
				((uint32_t*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_auIDs[6];
				((uint32_t*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_auIDs[7];

				((float*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_afWeights[0];
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_afWeights[1];
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_afWeights[2];
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_afWeights[3];
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_afWeights[4];
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_afWeights[5];
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_afWeights[6];
				((float*)mesh->m_pVerts)[index++] = mesh->m_pxBoneDatas[i].m_afWeights[7];
			}
		}

		mesh->m_pxBufferLayout->CalculateOffsetsAndStrides();

		return mesh;

		return nullptr;
	}

	void Mesh::GenerateNormals()
	{
		for (size_t i = 0; i < m_uNumIndices / 3; i++)
		{
			int a = m_puIndices[i * 3];
			int b = m_puIndices[i * 3 + 1];
			int c = m_puIndices[i * 3 + 2];

			glm::vec3 posA = m_pxVertexPositions[a];// * glm::vec3(10,10,10);
			glm::vec3 posB = m_pxVertexPositions[b];// * glm::vec3(10, 10, 10);
			glm::vec3 posC = m_pxVertexPositions[c];// * glm::vec3(10, 10, 10);

			glm::vec3 normal = glm::cross(posB - posA, posC - posA);
			m_pxNormals[a] += normal;
			m_pxNormals[b] += normal;
			m_pxNormals[c] += normal;
		}

		for (size_t i = 0; i < m_uNumVerts; i++)
		{
			m_pxNormals[i] = glm::normalize(m_pxNormals[i]);
		}
	}
	void Mesh::GenerateTangents()
	{
#if 0
		for (uint32_t i = 0; i < m_uNumIndices/3; i++)
		{
			unsigned int a = m_puIndices[i * 3];
			unsigned int b = m_puIndices[i * 3 + 1];
			unsigned int c = m_puIndices[i * 3 + 2];
			glm::vec4 tangent = GenerateTangent(a, b, c);
			m_pxTangents[a] += tangent;
			m_pxTangents[b] += tangent;
			m_pxTangents[c] += tangent;
		}
		for (uint32_t i = 0; i < m_uNumVerts; i++)
		{
			float handedness = m_pxTangents[i].w > 0 ? 1.f : -1.f;
			m_pxTangents[i].w = 0;
			m_pxTangents[i] = glm::normalize(m_pxTangents[i]);
			m_pxTangents[i].w = handedness;
		}
#endif
	}
	glm::vec4 Mesh::GenerateTangent(uint32_t a, uint32_t b, uint32_t c) {
		glm::vec3 ba = m_pxVertexPositions[b] - m_pxVertexPositions[a];
		glm::vec3 ca = m_pxVertexPositions[c] - m_pxVertexPositions[a];
		glm::vec2 tba = m_pxUVs[b] - m_pxUVs[a];
		glm::vec2 tca = m_pxUVs[c] - m_pxUVs[a];

		glm::mat2 texMatrix(tba, tca);
		texMatrix = glm::inverse(texMatrix);

		glm::vec3 tangent = ba * texMatrix[0][0] + ca * texMatrix[0][1];
		glm::vec3 binormal = ba * texMatrix[1][0] + ca * texMatrix[1][1];

		glm::vec3 normal = glm::cross(ba,ca);
		glm::vec3 biCross = glm::cross(tangent,normal);

		float handedness = glm::dot(biCross,binormal) < 0.f ? -1.f : 1.f;
		return glm::vec4(tangent.x,tangent.y,tangent.z,handedness);
	}
}