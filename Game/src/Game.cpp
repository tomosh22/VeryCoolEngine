#include "Game.h"
#include <imgui.h>



#include "VeryCoolEngine.h"

#include "VeryCoolEngine/BlockWorld/BlockWorld.h"

namespace VeryCoolEngine {

	const std::unordered_map<Block::FaceType, glm::ivec2> Block::atlasOffsets = {
		{ Block::FaceType::GrassFull, {0,0} },
		{ Block::FaceType::Stone, {1,0} },
		{ Block::FaceType::Dirt, {2,0} },
		{ Block::FaceType::GrassSide, {3,0} },
		{ Block::FaceType::WoodenPlanks, {4,0} },
		{ Block::FaceType::StoneSlab, {5,0} },
		{ Block::FaceType::PolishedStone, {6,0} },
		{ Block::FaceType::Brick, {7,0} },
		{ Block::FaceType::TNT, {8,0} },
	};

	Game::Game() {


		//m_pxBlockWorld = new BlockWorld();

		m_xMaterialMap.insert({ "rock2k", Material::Create("rock2k") });

		AddModel("ogre.fbx", Transform({ 0,0,0 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(1, 1, 1)
		));

		AddModel("otherFish.fbx", Transform({ 20,0,0 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(1, 1, 1)
		));


		AddModel("barrel.fbx", Transform({ 0,0,0 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(1, 1, 1)
		));

		AddModel("sphereSmooth.obj", m_xMaterialMap.at("rock2k"), Transform({0,0,0}, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(1, 1, 1)
		));

		AddModel("plane.obj", m_xMaterialMap.at("rock2k"), Transform({ 0,0,0 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(100, 1, 100)
		));
		
		_lights.push_back({
				50,200,50,100,
				0,1,0,1
		});

		_lights.push_back({
				50,110,50,50,
				1,1,1,1
			});


		_Camera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, float(VCE_GAME_WIDTH) / float(VCE_GAME_HEIGHT));
		

		_renderThreadCanStart = true;
		
		return;
	}

	//a little bit hacky
	VCEModel* Game::AddModel(const char* szFileName, Material* pxMaterial, const Transform& xTrans, uint32_t uMeshIndex /*= 0*/)
	{
		Mesh* mesh = Mesh::FromFile(szFileName, uMeshIndex);
		mesh->SetShader(m_pxMeshShader);
		mesh->m_pxMaterial = pxMaterial;

		
		mesh->m_xTransform = xTrans;
		mesh->m_xTransform.UpdateRotation();
		mesh->m_xTransform.UpdateMatrix();


		m_apxModels.push_back(new VCEModel());

		m_apxModels.back()->m_xTransform = xTrans;

		m_apxModels.back()->m_apxMeshes.push_back(mesh);

		return m_apxModels.back();
	}

	VCEModel* Game::AddModel(const char* szFileName, const Transform& xTrans)
	{
		m_apxModels.push_back(new VCEModel(szFileName));

		m_apxModels.back()->m_xTransform = xTrans;

		return m_apxModels.back();
	}

	

	

	Game::~Game() {}

	

	

	//declared in Application.h, defined by game
	void Application::GameLoop(float fDt) {
		Game* game = (Game*)Application::GetInstance();

		

		sceneMutex.lock();
		scene->Reset();

		scene->camera = &_Camera;
		scene->skybox = _pCubemap;

		for (RendererAPI::Light& light : _lights) {
			scene->lights[scene->numLights++] = light;
		}

		scene->m_axPipelineMeshes.insert({ "Skybox", std::vector<Mesh*>() });
		scene->m_axPipelineMeshes.at("Skybox").push_back(game->m_pxQuadModel->m_apxMeshes.back());


		scene->m_axPipelineMeshes.insert({ "Meshes", std::vector<Mesh*>() });
		
		scene->m_axPipelineMeshes.insert({ "SkinnedMeshes", std::vector<Mesh*>() });
		for (VCEModel* pxModel : m_apxModels) {
			if (pxModel->m_pxAnimation != nullptr) {
				//has an animation
				pxModel->m_pxAnimation->UpdateAnimation(fDt / 1000.f);
				std::vector<glm::mat4>& xAnimMats = pxModel->m_pxAnimation->GetFinalBoneMatrices();
				for (Mesh* pxMesh : pxModel->m_apxMeshes) {
					for (uint32_t i = 0; i < pxMesh->m_xBoneMats.size(); i++) {
						pxMesh->m_xBoneMats.at(i) = xAnimMats.at(i);
					}
					scene->m_axPipelineMeshes.at("SkinnedMeshes").push_back(pxMesh);
				}
			}
			else {
				//does not have an animation
				for (Mesh* pxMesh : pxModel->m_apxMeshes) {
					//hacky way to make sure this mesh belongs in this pipeline
					if(pxMesh->m_pxMaterial != nullptr)
						scene->m_axPipelineMeshes.at("Meshes").push_back(pxMesh);
				}
			}
		}

		scene->m_axPipelineMeshes.insert({ "GBuffer", std::vector<Mesh*>() });
		for (VCEModel* model : game->m_apxModels)
			for(Mesh* pxMesh : model->m_apxMeshes)
			scene->m_axPipelineMeshes.at("GBuffer").push_back(pxMesh);

		for (RendererAPI::Light& light : _lights) {
			scene->lights[scene->numLights++] = light;
		}

		RendererAPI::Light camLight{
				_Camera.GetPosition().x,_Camera.GetPosition().y,_Camera.GetPosition().z,100,
				1,1,1,1
		};
		scene->lights[scene->numLights++] = camLight;

		scene->ready = true;
		sceneMutex.unlock();



		//#TODO reimplement this
		//bool rState = Input::IsKeyPressed(VCE_KEY_R);
		//if (Input::IsKeyPressed(VCE_KEY_R) && prevRState != rState) {
		//	Chunk::seed = rand();
		//	game->_chunks.clear();
		//	game->m_pxInstanceMesh->m_axInstanceData.clear();
		//	game->m_pxInstanceMesh->m_uNumInstances = 0;
		//	game->_instanceMats.clear();
		//	game->_instanceQuats.clear();
		//	game->_instancePositions.clear();
		//	game->_instanceOffsets.clear();
		//	game->_instanceAOValues.clear();
		//	game->GenerateChunks();
		//	
		//	game->UploadChunks();
		//	scene->_functionsToRun.push_back([]() {//TODO call this from vulkan, currently leaves a load of glitchy block faces at the origin
		//		
		//		Game* game = (Game*)Application::GetInstance();
		//			game->m_pxInstanceMesh->GetVertexArray()->DisableVertexBuffer(game->m_pxInstanceMesh->GetVertexArray()->_VertexBuffers.back());
		//		
		//			

		//			VertexBuffer* instancedVertexBuffer = game->m_pxInstanceMesh->CreateInstancedVertexBuffer();
		//			game->m_pxInstanceMesh->GetVertexArray()->AddVertexBuffer(instancedVertexBuffer, true);
		//		
		//		});

		//}
		//prevRState = rState;
	}

	//extern definition (EntryPoint.h)
	VeryCoolEngine::Application* VeryCoolEngine::CreateApplication() {
		return new Game();
	}
}


