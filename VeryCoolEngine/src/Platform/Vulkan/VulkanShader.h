#pragma once
#include "VeryCoolEngine/Renderer/Shader.h"
#include "VulkanRenderer.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace VeryCoolEngine {
	class VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::string& vertex, const std::string& fragment, const std::string& geometry = "", const std::string& domain = "", const std::string& hull = "");
		~VulkanShader() { delete[] m_xInfos; }
		void ReloadShader() override;

		void Bind() override;

		void PlatformInit() override;

		void UploadMatrix4Uniform(const glm::mat4& matrix, const std::string& name) const override;
		void UploadVec3Uniform(const glm::vec3& matrix, const std::string& name) const override;
		void UploadIVec2Uniform(const glm::ivec2& ivec2, const std::string& name) const override;
		void UploadBoolUniform(const bool b, const std::string& name) const override;

		vk::ShaderModule xVertShaderModule;
		vk::ShaderModule xFragShaderModule;

		std::vector<char> m_vertShaderCode;
		std::vector<char> m_fragShaderCode;


		//credit Rich Davison
		void FillShaderStageCreateInfo(vk::GraphicsPipelineCreateInfo& info) const;
		int m_uStageCount = 2;//TODO support anything other than vert frag
		vk::PipelineShaderStageCreateInfo* m_xInfos = nullptr;

	private:
		vk::ShaderModule CreateShaderModule(const std::vector<char>& code);
		
	};

}

