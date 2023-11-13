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
			{ 0, 180, 0 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(10, 10, 10)
		), CollisionVolumeType::Sphere, false));
		m_apxGenericMeshes.push_back(AddTestMesh("sphereSmooth.obj", Transform(
			{ 50, 180, 80 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(10, 10, 10)
		), CollisionVolumeType::Sphere, false));
		m_apxGenericMeshes.push_back(AddTestMesh("sphereFlat.obj", Transform(
			{ 80,180,80 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(10, 10, 10)
		), CollisionVolumeType::Sphere, false));
		m_apxGenericMeshes.push_back(AddTestMesh("cubeFlat.obj", Transform(
			{ 80,180,10 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(10, 10, 10)
		), CollisionVolumeType::Cube, false));
		m_apxGenericMeshes.push_back(AddTestMesh("cubeSmooth.obj", Transform(
			{ 20,180,10 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(10, 10, 10)
		), CollisionVolumeType::Cube, false));
		m_apxGenericMeshes.push_back(AddTestMesh("sphereSmoothIco.obj", Transform(
			{ 50,180,50 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(10, 10, 10)
		), CollisionVolumeType::Sphere, false));
		m_apxGenericMeshes.push_back(AddTestMesh("sphereSmoothIcoLowPoly.obj", Transform(
			{ 50,220,50 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(10, 10, 10)
		), CollisionVolumeType::Sphere, false));

		

		

		
		_lights.push_back({
				50,200,50,100,
				0,1,0,1
		});

		_lights.push_back({
				50,110,50,50,
				1,1,1,1
			});

		//#TODO let client set skybox texture

		
		
		_Camera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, float(VCE_GAME_WIDTH) / float(VCE_GAME_HEIGHT));
		

		_renderThreadCanStart = true;
		
		return;
	}

	Mesh* Application::AddTestMesh(const char* szFileName, const Transform& xTrans, CollisionVolumeType type, bool bIsImmovable)
	{
		Mesh* mesh = Mesh::FromFile(szFileName);
		mesh->SetShader(m_pxMeshShader);
		mesh->SetTexture(m_pxGenericDiffuse);
		mesh->SetBumpMap(m_pxGenericBump);
		mesh->SetRoughnessTex(m_pxGenericRoughness);
		mesh->SetMetallicTex(m_pxGenericMetallic);
		mesh->SetHeightmapTex(m_pxGenericHeightmap);
		

		TextureDescriptorSpecification xMeshTexSpec;
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		xMeshTexSpec.m_aeSamplerStages.push_back({ nullptr, ShaderStageFragment });
		mesh->m_xTexDescSpec = xMeshTexSpec;
		
		physx::PxShape* shape;
		switch (type) {
		case CollisionVolumeType::Cube:
			shape = Physics::s_pxPhysics->createShape(physx::PxBoxGeometry(xTrans._scale.x, xTrans._scale.y, xTrans._scale.z), *Physics::s_pxMaterial);
			break;
		case CollisionVolumeType::Sphere:
			//assuming uniform scale
			shape = Physics::s_pxPhysics->createShape(physx::PxSphereGeometry(xTrans._scale.x), *Physics::s_pxMaterial);
			break;
		}
		physx::PxTransform pxTrans(xTrans._position.x, xTrans._position.y, xTrans._position.z);
		physx::PxRigidDynamic* body = Physics::s_pxPhysics->createRigidDynamic(pxTrans);
		body->attachShape(*shape);
		if (bIsImmovable) {
			body->setMass(0);
			body->setMassSpaceInertiaTensor(physx::PxVec3(0.f, 0.f, 0.f));
			body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
		}
		else {
			physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
		}
		Physics::s_pxScene->addActor(*body);

		mesh->m_pxActor = body;
		mesh->m_xScale = xTrans._scale;

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

		Physics::UpdatePhysics();

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


