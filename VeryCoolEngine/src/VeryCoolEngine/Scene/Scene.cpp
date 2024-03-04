#include "vcepch.h"
#include "Entity.h"
#include "Scene.h"
#include "VeryCoolEngine/Components/ModelComponent.h"
#include "VeryCoolEngine/Components/TransformComponent.h"
#include "VeryCoolEngine/Components/ColliderComponent.h"
#include "VeryCoolEngine/Components/ScriptComponent.h"
#include "VeryCoolEngine/Components/TerrainComponent.h"
#include "VeryCoolEngine/Application.h"
namespace VeryCoolEngine {

	enum class ComponentType {
		Transform,
		Model,
		Collider,
		Script,
		Terrain
	};
	std::unordered_map<std::string, ComponentType> g_xComponentNames = {
		{"TransformComponent", ComponentType::Transform},
		{"ModelComponent", ComponentType::Model},
		{"ColliderComponent", ComponentType::Collider},
		{"ScriptComponent", ComponentType::Script},
		{"TerrainComponent", ComponentType::Terrain},
	};

	std::unordered_map<std::string, Physics::CollisionVolumeType> g_xColliderNames = {

		{"OBB", Physics::CollisionVolumeType::OBB },
		{"Sphere", Physics::CollisionVolumeType::Sphere },
	};

	std::unordered_map<std::string, Physics::RigidBodyType> g_xRigidBodyNames = {

		{"Dynamic", Physics::RigidBodyType::Dynamic },
		{"Static", Physics::RigidBodyType::Static },
	};

	enum class ScriptBehaviourType {
		PlayerController
	};
	std::unordered_map<std::string, ScriptBehaviourType> g_xScriptBehaviourNames = {
		{"PlayerController", ScriptBehaviourType::PlayerController},
	};

	void Scene::LoadAssets(const std::string& strFilename) {
		Application* app = Application::GetInstance();
		std::ifstream xIn(strFilename);
		std::string strLine;
		while (std::getline(xIn, strLine)) {
			if (strLine == "Entity") {
				std::string strGuid;
				std::string strParentGuid;
				std::string strName;
				std::getline(xIn, strGuid);
				std::getline(xIn, strParentGuid);
				std::getline(xIn, strName);
				Entity xEntity(this, GUID(strtoull(strGuid.c_str(), nullptr, 10)), GUID(strtoull(strParentGuid.c_str(), nullptr, 10)), strName);
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
						std::string strVolumeType;
						std::string strRigidBodyType;
						std::getline(xIn, strVolumeType);
						std::getline(xIn, strRigidBodyType);
						xCollider.AddCollider(g_xColliderNames[strVolumeType], g_xRigidBodyNames[strRigidBodyType]);
					}
					break;
					case ComponentType::Model:
					{
						std::string strMeshGUID;
						std::string strMaterialGUID;
						std::getline(xIn, strMaterialGUID);
						std::getline(xIn, strMeshGUID);
						GUID xMeshGUID(strtoull(strMeshGUID.c_str(), nullptr, 10));
						GUID xMaterialGUID(strtoull(strMaterialGUID.c_str(), nullptr, 10));
						ModelComponent& xModel = xEntity.AddComponent<ModelComponent>(xMeshGUID, xMaterialGUID);
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
						case ScriptBehaviourType::PlayerController:
							xScript.SetBehaviour<PlayerController>();
							xScript.Instantiate(&xScript);
							m_xPlayerGuid = xEntity.GetGuid();
							xScript.m_pxScriptBehaviour->m_axGuidRefs.resize(uNumGuids);
							for (uint32_t i = 0; i < uNumGuids; i++)
								xScript.m_pxScriptBehaviour->m_axGuidRefs.at(i).m_uGuid = strtoull(astrGuids[i].c_str(), nullptr, 10);
							bool a = false;
							break;
						}
					}
					break;
					case ComponentType::Terrain:
					{
						std::string strHeightmapGUID;
						std::string strMaterialGUID;
						std::string strCoordinate;
						std::getline(xIn, strHeightmapGUID);
						std::getline(xIn, strMaterialGUID);
						std::getline(xIn, strCoordinate);
						std::stringstream xCoordinateStream(strCoordinate);
						std::string strX, strY;
						std::getline(xCoordinateStream, strX, ' ');
						std::getline(xCoordinateStream, strY, ' ');
						GUID xHeightmapGUID(strtoull(strHeightmapGUID.c_str(), nullptr, 10));
						GUID xMaterialGUID(strtoull(strMaterialGUID.c_str(), nullptr, 10));
						xEntity.AddComponent<TerrainComponent>(xHeightmapGUID, xMaterialGUID, std::stoi(strX), std::stoi(strY));
					}
					break;
					}
				}
			}
		}
		app->_pRenderer->InitialiseAssets();
	}

	Scene::Scene(const std::string& strFilename) {
		Application* app = Application::GetInstance();

		//#TO_TODO: will cause problems if i have more than one scene loaded at a time
		app->m_pxCurrentScene = this;

		

		LoadAssets(strFilename);

		

		m_xEditorCamera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, float(VCE_GAME_WIDTH) / float(VCE_GAME_HEIGHT));
		m_xGameCamera = Camera::BuildPerspectiveCamera(glm::vec3(0, 70, 5), 0, 0, 45, 1, 1000, float(VCE_GAME_WIDTH) / float(VCE_GAME_HEIGHT));
		for (ScriptComponent* pxScript : GetAllOfComponentType<ScriptComponent>())
			pxScript->OnCreate();
	}

	Scene::~Scene() {
		m_xRegistry.clear();
	}

	void Scene::Reset() {
		
	}


	//#TO_TODO: don't like this
	std::vector<ColliderComponent*> Scene::GetAllColliderComponents() {
		std::vector<ColliderComponent*> xRet;
		for (ColliderComponent* pxCol : GetAllOfComponentType<ColliderComponent>())
			xRet.push_back(pxCol);
		return xRet;
	}

	void Scene::Serialize(const std::string& strFilename) {
		VCE_TRACE("Serializing {}", strFilename.c_str());
		std::ofstream xOut(strFilename.c_str());
		
		for (auto [xGuid, xEntity] : m_xEntityMap) {
			xOut << "Entity\n";
			xEntity.Serialize(xOut);
			xOut << "EndEntity\n";
		}
		xOut.close();
	}

	Entity Scene::GetEntityByGuid(GUID xGuid) {
		return m_xEntityMap.at(xGuid.m_uGuid);
	}
	Entity Scene::GetEntityByGuid(GuidType uGuid) {
		return m_xEntityMap.at(uGuid);
	}
}