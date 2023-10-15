#include "vcepch.h"
#include "VulkanShader.h"

namespace VeryCoolEngine {
	VulkanShader::VulkanShader(const std::string& vertex, const std::string& fragment, const std::string& geometry, const std::string& domain, const std::string& hull)
	{

		m_vertShaderCode = VulkanRenderer::ReadFile(vertex.c_str());
		m_fragShaderCode = VulkanRenderer::ReadFile(fragment.c_str());

		

	}

	void VulkanShader::PlatformInit() {
		xVertShaderModule = CreateShaderModule(m_vertShaderCode);
		xFragShaderModule = CreateShaderModule(m_fragShaderCode);

		m_xInfos = new vk::PipelineShaderStageCreateInfo[m_uStageCount];

		//vert
		m_xInfos[0].stage = vk::ShaderStageFlagBits::eVertex;
		m_xInfos[0].module = xVertShaderModule;
		m_xInfos[0].pName = "main";

		//frag
		m_xInfos[1].stage = vk::ShaderStageFlagBits::eFragment;
		m_xInfos[1].module = xFragShaderModule;
		m_xInfos[1].pName = "main";
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

	//credit Rich Davison
	void VulkanShader::FillShaderStageCreateInfo(vk::GraphicsPipelineCreateInfo& info) const {
		info.setStageCount(m_uStageCount);
		info.setPStages(m_xInfos);
	}
}