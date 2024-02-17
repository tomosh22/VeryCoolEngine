#pragma once
#include "VeryCoolEngine/Renderer/Shader.h"
#include "VulkanRenderer.h"

namespace VeryCoolEngine {
	class VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::string& vertex, const std::string& fragment, const std::string& geometry = "", const std::string& domain = "", const std::string& hull = "");
		~VulkanShader() {
			VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();
			vk::Device xDevice = pxRenderer->GetDevice();
			xDevice.destroyShaderModule(xVertShaderModule);
			xDevice.destroyShaderModule(xFragShaderModule);
			xDevice.destroyShaderModule(xTescShaderModule);
			xDevice.destroyShaderModule(xTeseShaderModule);
			delete[] m_xInfos;
		}
		void ReloadShader() override;

		void Bind() override;

		void PlatformInit() override;

		void UploadMatrix4Uniform(const glm::mat4& matrix, const std::string& name) const override;
		void UploadVec3Uniform(const glm::vec3& matrix, const std::string& name) const override;
		void UploadIVec2Uniform(const glm::ivec2& ivec2, const std::string& name) const override;
		void UploadBoolUniform(const bool b, const std::string& name) const override;

		vk::ShaderModule xVertShaderModule;
		vk::ShaderModule xFragShaderModule;
		vk::ShaderModule xTescShaderModule;
		vk::ShaderModule xTeseShaderModule;

		std::vector<char> m_vertShaderCode;
		std::vector<char> m_fragShaderCode;
		std::vector<char> m_tescShaderCode;
		std::vector<char> m_teseShaderCode;


		//credit Rich Davison
		void FillShaderStageCreateInfo(vk::GraphicsPipelineCreateInfo& info) const;
		uint32_t m_uStageCount;//TODO support anything other than vert frag
		vk::PipelineShaderStageCreateInfo* m_xInfos = nullptr;

	private:
		vk::ShaderModule CreateShaderModule(const std::vector<char>& code);
		
	};

}

