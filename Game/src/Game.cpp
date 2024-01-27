#include "Game.h"
#include <imgui.h>



#include "VeryCoolEngine.h"

#include "VeryCoolEngine/BlockWorld/BlockWorld.h"

#include "VeryCoolEngine/Physics/Physics.h"

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
		m_xMaterialMap.insert({ "crystal2k", Material::Create("crystal2k") });

		//AddModel("ogre.fbx", Transform({ 0,200,0 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(1, 1, 1)), true);

		//AddModel("otherFish.fbx", Transform({ 20,200,0 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(1, 1, 1)), true);


		//AddModel("barrel.fbx", Transform({ 0,200,0 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(1, 1, 1)), true);

		VCEModel* pxSphere = AddModel("sphereSmooth.obj", m_xMaterialMap.at("rock2k"), Transform({10,50,10}, glm::vec3(10, 10, 10)));
		AddSphereCollisionVolumeToModel(pxSphere, 10);

		VCEModel* pxCube = AddModel("cubeFlat.obj", m_xMaterialMap.at("rock2k"), Transform({ -10,50,-10 }, glm::vec3(10, 10, 10)));
		AddBoxCollisionVolumeToModel(pxCube, pxCube->m_xScale);

		//blender doesn't UV map capsules so just using a stretched sphere instead
		VCEModel* pxCapsule = AddModel("sphereSmooth.obj", m_xMaterialMap.at("rock2k"), Transform({ 10,50,-10 }, glm::vec3(5, 10, 5)));
		AddCapsuleCollisionVolumeToModel(pxCapsule, 5,10);
		pxCapsule->m_pxRigidBody->setAngularLockAxisFactor(reactphysics3d::Vector3(0, 0, 0));

		VCEModel* pxPlane = AddModel("plane.obj", m_xMaterialMap.at("crystal2k"), Transform({ 0,0,0 }, glm::vec3(1000, 0.1, 1000)));
		AddBoxCollisionVolumeToModel(pxPlane, pxPlane->m_xScale);
		pxPlane->m_pxRigidBody->setType(reactphysics3d::BodyType::STATIC);
		
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
	VCEModel* Game::AddModel(const char* szFileName, Material* pxMaterial, Transform xTrans)
	{
		Mesh* mesh = Mesh::FromFile(szFileName);
		mesh->SetShader(m_pxMeshShader);
		mesh->m_pxMaterial = pxMaterial;

		m_apxModels.push_back(new VCEModel());

		VCEModel* pxModel = m_apxModels.back();

		xTrans.UpdateMatrix();
		pxModel->m_pxTransform = new reactphysics3d::Transform;

		reactphysics3d::Vector3 xPos = {xTrans.m_xPosition.x, xTrans.m_xPosition.y, xTrans.m_xPosition.z };
		pxModel->m_pxTransform->setPosition(xPos);

		reactphysics3d::Quaternion xQuat = { xTrans.m_xRotationQuat.x, xTrans.m_xRotationQuat.y, xTrans.m_xRotationQuat.z, xTrans.m_xRotationQuat.w };
		pxModel->m_pxTransform->setOrientation(xQuat);

		pxModel->m_xScale = xTrans.m_xScale;


		

		pxModel->m_apxMeshes.push_back(mesh);
		pxModel->m_strDirectory = szFileName;

		return pxModel;
	}

	VCEModel* Game::AddModel(const char* szFileName, Transform xTrans)
	{
		m_apxModels.push_back(new VCEModel(szFileName));

		VCEModel* pxModel = m_apxModels.back();

		xTrans.UpdateMatrix();
		pxModel->m_pxTransform = new reactphysics3d::Transform;
		if (pxModel->m_pxTransform == nullptr)
			pxModel->m_pxTransform = new reactphysics3d::Transform;
		pxModel->m_pxTransform->setFromOpenGL((reactphysics3d::decimal*)&xTrans.m_xMatrix[0][0]);

		pxModel->m_xScale = xTrans.m_xScale;

		pxModel->m_xScale = xTrans.m_xScale;

		return pxModel;
	}

	void Game::AddBoxCollisionVolumeToModel(VCEModel* pxModel, glm::vec3 xHalfExtents)
	{
		pxModel->m_bUsePhysics = true;

		pxModel->m_pxRigidBody = Physics::s_pxPhysicsWorld->createRigidBody(*pxModel->m_pxTransform);

		reactphysics3d::BoxShape* pxShape = Physics::s_xPhysicsCommon.createBoxShape(reactphysics3d::Vector3(xHalfExtents.x, xHalfExtents.y, xHalfExtents.z));
		reactphysics3d::Collider* pxCollider = pxModel->m_pxRigidBody->addCollider(pxShape, reactphysics3d::Transform::identity());
		pxModel->m_pxRigidBody->setType(reactphysics3d::BodyType::DYNAMIC);
	}

	void Game::AddSphereCollisionVolumeToModel(VCEModel* pxModel, float fRadius)
	{
		pxModel->m_bUsePhysics = true;

		pxModel->m_pxRigidBody = Physics::s_pxPhysicsWorld->createRigidBody(*pxModel->m_pxTransform);

		reactphysics3d::SphereShape* pxShape = Physics::s_xPhysicsCommon.createSphereShape(fRadius);
		reactphysics3d::Collider* pxCollider = pxModel->m_pxRigidBody->addCollider(pxShape, reactphysics3d::Transform::identity());
		pxModel->m_pxRigidBody->setType(reactphysics3d::BodyType::DYNAMIC);
	}

	void Game::AddCapsuleCollisionVolumeToModel(VCEModel* pxModel, float fRadius, float fHeight)
	{
		pxModel->m_bUsePhysics = true;

		pxModel->m_pxRigidBody = Physics::s_pxPhysicsWorld->createRigidBody(*pxModel->m_pxTransform);

		reactphysics3d::CapsuleShape* pxShape = Physics::s_xPhysicsCommon.createCapsuleShape(fRadius, fHeight);
		reactphysics3d::Collider* pxCollider = pxModel->m_pxRigidBody->addCollider(pxShape, reactphysics3d::Transform::identity());
		pxModel->m_pxRigidBody->setType(reactphysics3d::BodyType::DYNAMIC);
	}
	

	

	Game::~Game() {}

	

	

	//declared in Application.h, defined by game
	void Application::GameLoop(float fDt) {
		Game* game = (Game*)Application::GetInstance();
		



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


