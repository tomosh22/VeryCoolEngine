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
		void ReloadShader() override;

		void Bind() override;

		void UploadMatrix4Uniform(const glm::mat4& matrix, const std::string& name) const override;
		void UploadVec3Uniform(const glm::vec3& matrix, const std::string& name) const override;
		void UploadIVec2Uniform(const glm::ivec2& ivec2, const std::string& name) const override;
		void UploadBoolUniform(const bool b, const std::string& name) const override;

		vk::ShaderModule xVertShaderModule;
		vk::ShaderModule xFragShaderModule;

	private:
		vk::ShaderModule CreateShaderModule(const std::vector<char>& code);
		
	};

}

