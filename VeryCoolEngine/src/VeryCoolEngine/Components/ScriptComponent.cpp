#include "vcepch.h"
#include "ScriptComponent.h"

namespace VeryCoolEngine {

	void ScriptComponent::Serialize(std::ofstream& xOut) {
		xOut << "ScriptComponent\n";
		xOut << m_pxScriptBehaviour->GetBehaviourType() << '\n';
		xOut << m_pxScriptBehaviour->m_axGuidRefs.size() << '\n';
		for (GUID xGuid : m_pxScriptBehaviour->m_axGuidRefs)
			xOut << xGuid.m_uGuid << '\n';

	}

}