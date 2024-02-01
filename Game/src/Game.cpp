#include "Game.h"
#include <imgui.h>



#include "VeryCoolEngine.h"

#include "VeryCoolEngine/BlockWorld/BlockWorld.h"

#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>



namespace VeryCoolEngine {
	
	Game::Game() {

		m_xMaterialMap.insert({ "rock2k", Material::Create("rock2k") });
		m_xMaterialMap.insert({ "crystal2k", Material::Create("crystal2k") });

		//AddModel("ogre.fbx", Transform({ 0,200,0 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(1, 1, 1)), true);

		//AddModel("otherFish.fbx", Transform({ 20,200,0 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(1, 1, 1)), true);


		//AddModel("barrel.fbx", Transform({ 0,200,0 }, glm::quat_identity<float, glm::packed_highp>(), glm::vec3(1, 1, 1)), true);

		VCEModel* pxSphere = AddModel("sphereSmooth.obj", m_xMaterialMap.at("rock2k"), Transform({10,50,10}, glm::vec3(10, 10, 10)));
		Physics::AddSphereCollisionVolumeToModel(pxSphere, 10);

		VCEModel* pxCube = AddModel("cubeFlat.obj", m_xMaterialMap.at("rock2k"), Transform({ -10,50,-10 }, glm::vec3(10, 10, 10)));
		Physics::AddBoxCollisionVolumeToModel(pxCube, pxCube->m_xScale);

		//blender doesn't UV map capsules so just using a stretched sphere instead
		m_pxPlayerModel = AddModel("sphereSmooth.obj", m_xMaterialMap.at("rock2k"), Transform({ 10,50,-10 }, glm::vec3(5, 10, 5)));
		Physics::AddCapsuleCollisionVolumeToModel(m_pxPlayerModel, 5,10);
		m_pxPlayerModel->m_pxRigidBody->setAngularLockAxisFactor(reactphysics3d::Vector3(0, 0, 0));

		m_pxGroundPlane = AddModel("plane.obj", m_xMaterialMap.at("crystal2k"), Transform({ 0,0,0 }, glm::vec3(1000, 0.1, 1000)));
		Physics::AddBoxCollisionVolumeToModel(m_pxGroundPlane, m_pxGroundPlane->m_xScale);
		m_pxGroundPlane->m_pxRigidBody->setType(reactphysics3d::BodyType::STATIC);
		
		_lights.push_back({
				50,200,50,100,
				0,1,0,1
		});

		_lights.push_back({
				50,110,50,50,
				1,1,1,1
			});


		m_xGameCamera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, float(VCE_GAME_WIDTH) / float(VCE_GAME_HEIGHT));
		

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

	Game::~Game() {}

	

	

	//declared in Application.h, defined by game
	void Application::GameLoop(float fDt) {
		Game* game = (Game*)Application::GetInstance();
		
		glm::vec3 xCamPos = { game->m_pxPlayerModel->m_pxTransform->getPosition().x, game->m_pxPlayerModel->m_pxTransform->getPosition().y + 15, game->m_pxPlayerModel->m_pxTransform->getPosition().z };
		m_xGameCamera.SetPosition(xCamPos);
		m_xGameCamera.UpdateRotation();

		constexpr const float fMoveSpeed = 20;
		if (game->m_bPlayerIsOnFloor) {
			reactphysics3d::Vector3 xFinalVelocity(0, 0, 0);
			if (Input::IsKeyPressed(VCE_KEY_SPACE)) {
				xFinalVelocity += reactphysics3d::Vector3(0, 10, 0);
			}
			if (Input::IsKeyPressed(VCE_KEY_W)) {
				glm::mat4 rotation = glm::rotate(m_xGameCamera.GetYaw(), glm::vec3(0, 1, 0));
				glm::vec4 result = rotation * glm::vec4(0, 0, -1, 1);
				glm::vec3 xVelocity = glm::vec3(result.x, result.y, result.z) * fMoveSpeed;
				xFinalVelocity += reactphysics3d::Vector3(xVelocity.x, xVelocity.y, xVelocity.z);
			}
			if (Input::IsKeyPressed(VCE_KEY_S)) {
				glm::mat4 rotation = glm::rotate(m_xGameCamera.GetYaw(), glm::vec3(0, 1, 0));
				glm::vec4 result = rotation * glm::vec4(0, 0, -1, 1);
				result *= -1;
				glm::vec3 xVelocity = glm::vec3(result.x, result.y, result.z) * fMoveSpeed;
				xFinalVelocity += reactphysics3d::Vector3(xVelocity.x, xVelocity.y, xVelocity.z);
			}
			if (Input::IsKeyPressed(VCE_KEY_A)) {
				glm::mat4 rotation = glm::rotate(m_xGameCamera.GetYaw(), glm::vec3(0, 1, 0));
				glm::vec4 result = rotation * glm::vec4(-1, 0, 0, 1);
				glm::vec3 xVelocity = glm::vec3(result.x, result.y, result.z) * fMoveSpeed;
				xFinalVelocity += reactphysics3d::Vector3(xVelocity.x, xVelocity.y, xVelocity.z);
			}
			if (Input::IsKeyPressed(VCE_KEY_D)) {
				glm::mat4 rotation = glm::rotate(m_xGameCamera.GetYaw(), glm::vec3(0, 1, 0));
				glm::vec4 result = rotation * glm::vec4(-1, 0, 0, 1);
				result *= -1;
				glm::vec3 xVelocity = glm::vec3(result.x, result.y, result.z) * fMoveSpeed;
				xFinalVelocity += reactphysics3d::Vector3(xVelocity.x, xVelocity.y, xVelocity.z);
			}
			game->m_pxPlayerModel->m_pxRigidBody->setLinearVelocity(xFinalVelocity);
		}
	}

	void Application::CollisionCallback(VCEModel* pxModel1, VCEModel* pxModel2, Physics::CollisionEventType eType) {
		Game* pxGame = (Game*)Application::GetInstance();
		if ((pxModel1 == pxGame->m_pxPlayerModel && pxModel2 == pxGame->m_pxGroundPlane) || (pxModel2 == pxGame->m_pxPlayerModel && pxModel1 == pxGame->m_pxGroundPlane)) {
			if(eType == Physics::CollisionEventType::Start)
				pxGame->m_bPlayerIsOnFloor = true;
			else if (eType == Physics::CollisionEventType::Exit)
				pxGame->m_bPlayerIsOnFloor = false;
		}
			
	}

	//extern definition (EntryPoint.h)
	VeryCoolEngine::Application* VeryCoolEngine::CreateApplication() {
		return new Game();
	}
}


