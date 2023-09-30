#include "vcepch.h"
#include "VulkanShader.h"

namespace VeryCoolEngine {
	VulkanShader::VulkanShader(const std::string& vertex, const std::string& fragment, const std::string& geometry, const std::string& domain, const std::string& hull)
	{

		std::vector<char> vertShaderCode = VulkanRenderer::ReadFile(vertex.c_str());
		std::vector<char> fragShaderCode = VulkanRenderer::ReadFile(fragment.c_str());

		xVertShaderModule = CreateShaderModule(vertShaderCode);
		xFragShaderModule = CreateShaderModule(fragShaderCode);

	}
	void VulkanShader::ReloadShader()
	{
	}
	void VulkanShader::Bind()
	{
	}
	void VulkanShader::UploadMatrix4Uniform(const glm::mat4& matrix, const std::string& name) const
	{
	}
	void VulkanShader::UploadVec3Uniform(const glm::vec3& matrix, const std::string& name) const
	{
	}
	void VulkanShader::UploadIVec2Uniform(const glm::ivec2& ivec2, const std::string& name) const
	{
	}
	void VulkanShader::UploadBoolUniform(const bool b, const std::string& name) const
	{
	}
	vk::ShaderModule VulkanShader::CreateShaderModule(const std::vector<char>& code)
	{
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		vk::ShaderModule module;
		module = VulkanRenderer::GetInstance()->GetDevice().createShaderModule(createInfo);
		return module;
	}
}