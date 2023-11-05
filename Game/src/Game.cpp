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


		m_pxBlockWorld = new BlockWorld();
		
		

		m_apxGenericMeshes.push_back(AddTestMesh("sphereSmooth.obj", Transform(
			{ 50, 80, 80 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(10, 10, 10)
		)));
		m_apxGenericMeshes.push_back(AddTestMesh("sphereFlat.obj", Transform(
			{ 80,80,80 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(10, 10, 10)
		)));
		m_apxGenericMeshes.push_back(AddTestMesh("cubeFlat.obj", Transform(
			{ 80,80,10 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(10, 10, 10)
		)));
		m_apxGenericMeshes.push_back(AddTestMesh("cubeSmooth.obj", Transform(
			{ 20,80,10 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(10, 10, 10)
		)));
		m_apxGenericMeshes.push_back(AddTestMesh("sphereSmoothIco.obj", Transform(
			{ 50,80,50 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(10, 10, 10)
		)));
		m_apxGenericMeshes.push_back(AddTestMesh("sphereSmoothIcoLowPoly.obj", Transform(
			{ 50,120,50 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(10, 10, 10)
		)));

		

		
		_lights.push_back({
				50,200,50,100,
				0,1,0,1
		});

		_lights.push_back({
				50,110,50,50,
				1,1,1,1
			});

		//#TODO let client set skybox texture

		
		
		_Camera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, 1280.f / 720.f);
		

		_renderThreadCanStart = true;
		
		return;
	}

	Mesh* Game::AddTestMesh(const char* szFileName, const Transform& xTrans)
	{
		Mesh* mesh = Mesh::FromFile(szFileName);
		mesh->SetShader(m_pxMeshShader);
		mesh->SetTexture(Texture2D::Create("crystal2k/violet_crystal_43_04_diffuse.jpg", false));
		mesh->SetBumpMap(Texture2D::Create("crystal2k/violet_crystal_43_04_normal.jpg", false));
		mesh->SetRoughnessTex(Texture2D::Create("crystal2k/violet_crystal_43_04_roughness.jpg", false));
		mesh->SetMetallicTex(Texture2D::Create("crystal2k/violet_crystal_43_04_metallic.jpg", false));
		mesh->SetHeightmapTex(Texture2D::Create("crystal2k/violet_crystal_43_04_height.jpg", false));
		

		TextureDescriptorSpecification xMeshTexSpec;
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		mesh->m_xTexDescSpec = xMeshTexSpec;
		
		mesh->m_xTransform = xTrans;
		mesh->m_xTransform.UpdateRotation();
		mesh->m_xTransform.UpdateMatrix();

		_meshes.push_back(mesh);
		return mesh;
	}

	

	

	Game::~Game() {}

	

	

	//declared in Application.h, defined by game
	void Application::GameLoop() {
		Game* game = (Game*)Application::GetInstance();
		printf("game loop\n");


		sceneMutex.lock();
		scene->Reset();

		scene->camera = &_Camera;
		scene->skybox = _pCubemap;

		for (Mesh* mesh : _meshes) {
			scene->meshes.push_back(mesh);
		}

		for (Renderer::Light& light : _lights) {
			scene->lights[scene->numLights++] = light;
		}

		scene->m_axPipelineMeshes.insert({ "Skybox", std::vector<Mesh*>() });
		scene->m_axPipelineMeshes.at("Skybox").push_back(game->m_pxQuadMesh);

		scene->m_axPipelineMeshes.insert({ "Blocks", std::vector<Mesh*>() });
		scene->m_axPipelineMeshes.at("Blocks").push_back(game->m_pxInstanceMesh);

		scene->m_axPipelineMeshes.insert({ "Meshes", std::vector<Mesh*>() });

		for(Mesh* mesh : game->m_apxGenericMeshes)
			scene->m_axPipelineMeshes.at("Meshes").push_back(mesh);

		scene->m_axPipelineMeshes.insert({ "GBuffer", std::vector<Mesh*>() });

		for (Mesh* mesh : game->m_apxGenericMeshes)
			scene->m_axPipelineMeshes.at("GBuffer").push_back(mesh);

		for (Renderer::Light& light : _lights) {
			scene->lights[scene->numLights++] = light;
		}

		Renderer::Light camLight{
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


