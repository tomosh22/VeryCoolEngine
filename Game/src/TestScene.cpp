#include "vcepch.h"
#include "TestScene.h"
#include "VeryCoolEngine/Components/ModelComponent.h"
#include "VeryCoolEngine/Components/TransformComponent.h"
#include "VeryCoolEngine/Components/ColliderComponent.h"
#include "VeryCoolEngine/Components/ScriptComponent.h"
#include "VeryCoolEngine/Application.h"

namespace VeryCoolEngine {

	enum class ComponentType {
		Transform,
		Model,
		Collider,
		Script
	};
	std::unordered_map<std::string, ComponentType> g_xComponentNames = {
		{"TransformComponent", ComponentType::Transform},
		{"ModelComponent", ComponentType::Model},
		{"ColliderComponent", ComponentType::Collider},
		{"ScriptComponent", ComponentType::Script},
	};

	std::unordered_map<std::string, Physics::CollisionVolumeType> g_xColliderNames = {

		{"OBB", Physics::CollisionVolumeType::OBB },
		{"Sphere", Physics::CollisionVolumeType::Sphere },
	};

	enum class ScriptBehaviourType {
		TestScriptBehaviour
	};
	std::unordered_map<std::string, ScriptBehaviourType> g_xScriptBehaviourNames = {
		{"TestScriptBehaviour", ScriptBehaviourType::TestScriptBehaviour},
	};

	TestScene::TestScene()
	{
		Application* app = Application::GetInstance();

		//#TO_TODO: will cause problems if i have more than one scene loaded at a time
		app->m_pxCurrentScene = this;

		std::ifstream xIn("TestScene.vcescene");
		std::string strLine;
		while (std::getline(xIn, strLine)) {
			if (strLine == "Entity") {
				Entity xEntity(this);
				std::string strGuid;
				std::getline(xIn, strGuid);
				while (std::getline(xIn, strLine)) {
					if (strLine == "EndEntity")break;
					switch (g_xComponentNames[strLine]) {
					case ComponentType::Transform:
					{
						std::string strPosition;
						std::string strOrientation;
						std::string strScale;
						std::getline(xIn, strPosition);
						std::getline(xIn, strOrientation);
						std::getline(xIn, strScale);
						TransformComponent& xTrans = xEntity.GetComponent<TransformComponent>();

						std::string strPosX, strPosY, strPosZ;
						std::stringstream xPosStream(strPosition);
						std::getline(xPosStream, strPosX, ' ');
						std::getline(xPosStream, strPosY, ' ');
						std::getline(xPosStream, strPosZ, ' ');
						glm::vec3 xPos = { std::atof(strPosX.c_str()), std::atof(strPosY.c_str()), std::atof(strPosZ.c_str()) };
						xTrans.SetPosition(xPos);

						std::string strOriX, strOriY, strOriZ, strOriW;
						std::stringstream xOriStream(strOrientation);
						std::getline(xOriStream, strOriX, ' ');
						std::getline(xOriStream, strOriY, ' ');
						std::getline(xOriStream, strOriZ, ' ');
						std::getline(xOriStream, strOriW, ' ');
						glm::quat xOri;
						xOri.x = std::atof(strOriX.c_str());
						xOri.y = std::atof(strOriY.c_str());
						xOri.z = std::atof(strOriZ.c_str());
						xOri.w = std::atof(strOriW.c_str());
						xTrans.SetRotation(xOri);

						std::string strScaleX, strScaleY, strScaleZ;
						std::stringstream xScaleStream(strScale);
						std::getline(xScaleStream, strScaleX, ' ');
						std::getline(xScaleStream, strScaleY, ' ');
						std::getline(xScaleStream, strScaleZ, ' ');
						glm::vec3 xScale = { std::atof(strScaleX.c_str()), std::atof(strScaleY.c_str()), std::atof(strScaleZ.c_str()) };
						xTrans.SetScale(xScale);
					}
						break;
					case ComponentType::Collider:
					{
						ColliderComponent& xCollider = xEntity.AddComponent<ColliderComponent>();
						std::string strType;
						std::getline(xIn, strType);
						xCollider.AddCollider(g_xColliderNames[strType]);
					}
						break;
					case ComponentType::Model:
					{
						std::string strFile;
						std::string strMaterial;
						std::getline(xIn, strMaterial);
						std::getline(xIn, strFile);
						ModelComponent& xModel = xEntity.AddComponent<ModelComponent>(strFile, strMaterial.c_str());
					}
					break;
					case ComponentType::Script:
					{
						std::string strBehaviourType;
						std::string strNumGuids;
						std::vector<std::string> astrGuids;
						std::getline(xIn, strBehaviourType);
						std::getline(xIn, strNumGuids);
						uint32_t uNumGuids = std::atoi(strNumGuids.c_str());
						astrGuids.resize(uNumGuids);
						for (uint32_t i = 0; i < uNumGuids; i++) {
							std::getline(xIn, astrGuids.at(i));
						}
						ScriptComponent& xScript = xEntity.AddComponent<ScriptComponent>();
						switch (g_xScriptBehaviourNames[strBehaviourType]) {
						case ScriptBehaviourType::TestScriptBehaviour:
							xScript.SetBehaviour<TestScriptBehaviour>();
							xScript.Instantiate(&xScript);
							m_xPlayerGuid = xEntity.GetGuid();
							xScript.m_pxScriptBehaviour->m_axGuidRefs.resize(uNumGuids);
							for(uint32_t i = 0; i < uNumGuids; i++)
								xScript.m_pxScriptBehaviour->m_axGuidRefs.at(i).m_uGuid = strtoul(astrGuids[i].c_str(), nullptr, 10);
							bool a = false;
							break;
						}
					}
					break;
					}
				}
			}
		}
#if 0
		std::string strName;

		Entity xPlayerEntity(this);
		strName = "sphereSmooth.obj";
		ModelComponent& xPlayerModel = xPlayerEntity.AddComponent<ModelComponent>(strName, "rock2k");
		ScriptComponent& xPlayerScript = xPlayerEntity.AddComponent<ScriptComponent>();
		xPlayerScript.SetBehaviour<TestScriptBehaviour>();
		xPlayerScript.Instantiate(&xPlayerScript);
		m_xPlayerGuid = xPlayerEntity.GetGuid();
		

		strName = "sphereSmooth.obj";
		Entity xSphereEntity(this);
		ModelComponent& xSphereModel = xSphereEntity.AddComponent<ModelComponent>(strName, "rock2k");
		m_xSphereGuid = xSphereEntity.GetGuid();


		strName = "plane.obj";
		Entity xPlaneEntity(this);
		ModelComponent& xPlaneModel = xPlaneEntity.AddComponent<ModelComponent>(strName, "crystal2k");
		m_xPlaneGuid = xPlaneEntity.GetGuid();
		xPlayerScript.m_pxScriptBehaviour->m_axGuidRefs.push_back(xPlaneEntity.GetGuid());
#endif

		app->_pRenderer->InitialiseAssets();
		
		m_xEditorCamera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, float(VCE_GAME_WIDTH) / float(VCE_GAME_HEIGHT));

		Reset();

		
	}

	void TestScene::Reset() {
		Application* app = Application::GetInstance();

#if 0
		Physics::ResetPhysics();

		Entity xPlayerEntity = GetEntityByGuid(m_xPlayerGuid);
		TransformComponent& xPlayerTrans = xPlayerEntity.GetComponent<TransformComponent>();
		xPlayerTrans.SetPosition({ 5,50,5 });
		xPlayerTrans.SetScale({ 10,10,10 });
		ColliderComponent& xPlayerCollider = xPlayerEntity.AddOrReplaceComponent<ColliderComponent>();
		xPlayerCollider.AddCollider(Physics::CollisionVolumeType::OBB);
		ScriptComponent& xPlayerScript = xPlayerEntity.GetComponent<ScriptComponent>();
		xPlayerScript.OnCreate();
		

		Entity xSphereEntity = GetEntityByGuid(m_xSphereGuid);
		TransformComponent& xSphereTrans = xSphereEntity.GetComponent<TransformComponent>();
		xSphereTrans.SetPosition({ 25,5,25 });
		xSphereTrans.SetScale({ 10,10,10 });
		ColliderComponent& xSphereCollider = xSphereEntity.AddOrReplaceComponent<ColliderComponent>();
		xSphereCollider.AddCollider(Physics::CollisionVolumeType::Sphere);
		


		Entity xPlaneEntity = GetEntityByGuid(m_xPlaneGuid);
		TransformComponent& xPlaneTrans = xPlaneEntity.GetComponent<TransformComponent>();
		xPlaneTrans.SetPosition({ 1,1,1 });
		xPlaneTrans.SetScale({ 1000,1,1000 });
		ColliderComponent& xPlaneCollider = xPlaneEntity.AddOrReplaceComponent<ColliderComponent>();
		xPlaneCollider.AddCollider(Physics::CollisionVolumeType::OBB);
		xPlaneCollider.GetRigidBody()->setType(reactphysics3d::BodyType::STATIC);
#endif
		

		m_xGameCamera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, float(VCE_GAME_WIDTH) / float(VCE_GAME_HEIGHT));


	}
}