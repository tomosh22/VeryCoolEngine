#pragma once

#include "VeryCoolEngine.h"
#include "VeryCoolEngine/Application.h"
#include <glm/glm.hpp>

namespace VeryCoolEngine {

	class Block {
	public:
		enum class BlockType {
			Cobblestone,
			Stone,
			Dirt,
			Grass,
			WoodenPlanks,
			StoneSlab,
			PolishedStone,
			Brick,
			TNT
		};
		static const std::unordered_map<BlockType, glm::ivec2> atlasOffsets;

		Block(const glm::vec3& position, BlockType type) {

			Application* app = Application::GetInstance();

			Transform posX;
			posX._yaw = 90;
			posX._position = position;
			posX._position.x -= 0.5;
			posX.UpdateRotation();
			posX.UpdateMatrix();
			app->_instanceMats.push_back(posX._matrix);

			Transform negX;
			negX._yaw = 270;
			negX._position = position;
			negX._position.x += 0.5;
			negX.UpdateRotation();
			negX.UpdateMatrix();
			app->_instanceMats.push_back(negX._matrix);

			Transform posY;
			posY._roll = 270;
			posY._position = position;
			posY._position.y -= 0.5;
			posY.UpdateRotation();
			posY.UpdateMatrix();
			app->_instanceMats.push_back(posY._matrix);

			Transform negY;
			negY._roll = 90;
			negY._position = position;
			negY._position.y += 0.5;
			negY.UpdateRotation();
			negY.UpdateMatrix();
			app->_instanceMats.push_back(negY._matrix);

			Transform posZ;
			posZ._position = position;
			posZ._position.z -= 0.5;
			posZ.UpdateRotation();
			posZ.UpdateMatrix();
			app->_instanceMats.push_back(posZ._matrix);

			Transform negZ;
			negZ._yaw = 180;
			negZ._position = position;
			negZ._position.z += 0.5;
			negZ.UpdateRotation();
			negZ.UpdateMatrix();
			app->_instanceMats.push_back(negZ._matrix);

			app->_instanceOffsets.push_back(atlasOffsets.find(type)->second);

			app->_numInstances+=6;
			return;
			//#todo this is disgustingly inefficient
			//should be generating one cube face
			//that's instanced
			_pPosX = Mesh::GenerateCubeFace();
			_pPosX->transform._yaw = 90;
			_pPosX->transform._position = position;
			_pPosX->transform._position.x -= 0.5;

			_pNegX = Mesh::GenerateCubeFace();
			_pNegX->transform._yaw = 270;
			_pNegX->transform._position = position;
			_pNegX->transform._position.x += 0.5;

			_pPosY = Mesh::GenerateCubeFace();
			_pPosY->transform._roll = 270;
			_pPosY->transform._position = position;
			_pPosY->transform._position.y -= 0.5;

			_pNegY = Mesh::GenerateCubeFace();
			_pNegY->transform._roll = 90;
			_pNegY->transform._position = position;
			_pNegY->transform._position.y += 0.5;

			_pPosZ = Mesh::GenerateCubeFace();
			_pPosZ->transform._position = position;
			_pPosZ->transform._position.z -= 0.5;

			_pNegZ = Mesh::GenerateCubeFace();
			_pNegZ->transform._yaw = 180;
			_pNegZ->transform._position = position;
			_pNegZ->transform._position.z += 0.5;

			_pPosX->SetShader(app->_shaders[0]);
			_pNegX->SetShader(app->_shaders[0]);
			_pPosY->SetShader(app->_shaders[0]);
			_pNegY->SetShader(app->_shaders[0]);
			_pPosZ->SetShader(app->_shaders[0]);
			_pNegZ->SetShader(app->_shaders[0]);

			_pPosX->SetTexture(app->_textures[0]);
			_pNegX->SetTexture(app->_textures[0]);
			_pPosY->SetTexture(app->_textures[0]);
			_pNegY->SetTexture(app->_textures[0]);
			_pPosZ->SetTexture(app->_textures[0]);
			_pNegZ->SetTexture(app->_textures[0]);

			_pPosX->customUniform = atlasOffsets.find(type)->second;
			_pNegX->customUniform = atlasOffsets.find(type)->second;
			_pPosY->customUniform = atlasOffsets.find(type)->second;
			_pNegY->customUniform = atlasOffsets.find(type)->second;
			_pPosZ->customUniform = atlasOffsets.find(type)->second;
			_pNegZ->customUniform = atlasOffsets.find(type)->second;

			//#todo free instanceData
			_pPosX->instanceData = new glm::ivec2;
			*(glm::ivec2*)(_pPosX->instanceData) = atlasOffsets.find(type)->second;
			_pPosX->instanceDataType = ShaderDataType::Int2;
			_pPosX->instanceDataName = "_aAtlasOffset";

			_pNegX->instanceData = new glm::ivec2;
			*(glm::ivec2*)(_pNegX->instanceData) = atlasOffsets.find(type)->second;
			_pNegX->instanceDataType = ShaderDataType::Int2;
			_pNegX->instanceDataName = "_aAtlasOffset";

			_pPosY->instanceData = new glm::ivec2;
			*(glm::ivec2*)(_pPosY->instanceData) = atlasOffsets.find(type)->second;
			_pPosY->instanceDataType = ShaderDataType::Int2;
			_pPosY->instanceDataName = "_aAtlasOffset";

			_pNegY->instanceData = new glm::ivec2;
			*(glm::ivec2*)(_pNegY->instanceData) = atlasOffsets.find(type)->second;
			_pNegY->instanceDataType = ShaderDataType::Int2;
			_pNegY->instanceDataName = "_aAtlasOffset";

			_pPosZ->instanceData = new glm::ivec2;
			*(glm::ivec2*)(_pPosZ->instanceData) = atlasOffsets.find(type)->second;
			_pPosZ->instanceDataType = ShaderDataType::Int2;
			_pPosZ->instanceDataName = "_aAtlasOffset";

			_pNegZ->instanceData = new glm::ivec2;
			*(glm::ivec2*)(_pNegZ->instanceData) = atlasOffsets.find(type)->second;
			_pNegZ->instanceDataType = ShaderDataType::Int2;
			_pNegZ->instanceDataName = "_aAtlasOffset";


			app->_meshes.push_back(_pPosX);
			app->_meshes.push_back(_pNegX);
			app->_meshes.push_back(_pPosY);
			app->_meshes.push_back(_pNegY);
			app->_meshes.push_back(_pPosZ);
			app->_meshes.push_back(_pNegZ);
		}
		Mesh* _pPosX;
		Mesh* _pNegX;
		Mesh* _pPosY;
		Mesh* _pNegY;
		Mesh* _pPosZ;
		Mesh* _pNegZ;
	};

	class Game : public Application {
	public:
		Game();
		~Game();

		std::vector<Block> _blocks;
	};


}
