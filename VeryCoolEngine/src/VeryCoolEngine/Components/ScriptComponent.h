#pragma once
#include "TransformComponent.h"
#include "VeryCoolEngine/Scene/Entity.h"
namespace VeryCoolEngine {

	class ScriptBehaviour {
	public:
		virtual void OnCreate() = 0;
		virtual void OnUpdate(float fDt) = 0;
	};
	class TestScriptBehaviour : public ScriptBehaviour {
	public:
		virtual void OnCreate() override {
			VCE_TRACE("I've been created");
		}
		virtual void OnUpdate(float fDt) override {
			VCE_TRACE("I've been updated");
		}
	};
	class ScriptComponent
	{
	public:

		ScriptComponent(TransformComponent& xTrans, Entity* xEntity) {};

		void(*Instantiate)(ScriptBehaviour*&);
		void(*OnCreate)(ScriptBehaviour*&);
		void(*OnUpdate)(ScriptBehaviour*&, float);

		ScriptBehaviour* m_pxScriptBehaviour = nullptr;

		template<typename T>
		void SetBehaviour() {
			Instantiate = [](ScriptBehaviour*& pxScriptBehaviour) {pxScriptBehaviour = new T; };
			OnCreate = [](ScriptBehaviour*& pxScriptBehaviour) {dynamic_cast<T*>(pxScriptBehaviour)->OnCreate();};
			OnUpdate = [](ScriptBehaviour*& pxScriptBehaviour, float fDt) {dynamic_cast<T*>(pxScriptBehaviour)->OnUpdate(fDt);};
		}
	};
}


