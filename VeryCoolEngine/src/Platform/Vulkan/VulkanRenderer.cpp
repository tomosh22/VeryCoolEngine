#include "vcepch.h"
#include "VulkanRenderer.h"
#include "Platform/Windows/WindowsWindow.h"

#include "Platform/Vulkan/VulkanMesh.h"

#include "Platform/Vulkan/VulkanManagedUniformBuffer.h"
#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanPipelineBuilder.h"
#include "Platform/Vulkan/VulkanShader.h"

#include "VulkanPipelineBuilder.h"
#include "VulkanDescriptorSetLayoutBuilder.h"

#include <imgui.h>
#include "backends/imgui_impl_vulkan.h"

#include "VeryCoolEngine/Renderer/RendererAPI.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

#include "VulkanMaterial.h"


using namespace VeryCoolEngine;

VulkanRenderer* VulkanRenderer::s_pInstance = nullptr;

VulkanRenderer::VulkanRenderer() {
	InitWindow();
	InitVulkan();

	m_pxRendererAPI = new RendererAPI;

	m_pxCopyToFramebufferCommandBuffer = new VulkanCommandBuffer;
	m_pxSkyboxCommandBuffer = new VulkanCommandBuffer;
	m_pxOpaqueMeshesCommandBuffer = new VulkanCommandBuffer;
	m_pxSkinnedMeshesCommandBuffer = new VulkanCommandBuffer;




#ifdef VCE_DEFERRED_SHADING
	RendererAPI::s_xGBufferTargetSetup = CreateGBufferTarget();
#endif
	m_xTargetSetups.insert({ "RenderToTexture", CreateRenderToTextureTarget() });
	m_xTargetSetups.insert({ "CopyToFramebuffer", CreateFramebufferTarget() });
}

void VulkanRenderer::InitWindow() {
	Application* app = Application::GetInstance();
	((WindowsWindow*)app->_window)->Init(WindowProperties());
}

void VulkanRenderer::InitVulkan() {
	VulkanRenderer::s_pInstance = this;
	Application* app = Application::GetInstance();

	BoilerplateInit();


	
	for (Mesh* pMesh : app->_meshes) {
		pMesh->PlatformInit();
		pMesh->GetShader()->PlatformInit();
		if(pMesh->m_pxMaterial != nullptr)
			pMesh->m_pxMaterial->PlatformInit();
	}
	app->_pCameraUBO = ManagedUniformBuffer::Create(sizeof(glm::mat4) * 3 + sizeof(glm::vec4), MAX_FRAMES_IN_FLIGHT, 0);
	app->_pLightUBO = ManagedUniformBuffer::Create(sizeof(RendererAPI::Light) * RendererAPI::g_uMaxLights, MAX_FRAMES_IN_FLIGHT, 1);
	app->m_pxMiscMeshRenderDataUBO = ManagedUniformBuffer::Create(sizeof(Application::MeshRenderData), MAX_FRAMES_IN_FLIGHT, 2);

	for (Shader* pxShader : app->_shaders) pxShader->PlatformInit();
	//for (Texture* pxTex : app->_textures) pxTex->PlatformInit();

	for (auto it = app->m_xPipelineSpecs.begin(); it != app->m_xPipelineSpecs.end(); it++) {
		m_xPipelines.insert({ it->first,VulkanPipelineBuilder::FromSpecification(it->second) });
	}

#pragma region newpipelines
	
	if (!app->m_pxSkinnedMeshShader->m_bInitialised)
		app->m_pxSkinnedMeshShader->PlatformInit();
	if (!app->m_pxExampleSkinnedMesh->m_bInitialised)
		app->m_pxExampleSkinnedMesh->PlatformInit();

	VulkanPipelineBuilder xPipelineBuilder = VulkanPipelineBuilder("SkinnedMeshes")
		.WithVertexInputState(dynamic_cast<VulkanMesh*>(app->m_pxExampleSkinnedMesh)->m_xVertexInputState)
		.WithTopology(vk::PrimitiveTopology::eTriangleList)
		.WithShader(*dynamic_cast<VulkanShader*>(app->m_pxSkinnedMeshShader))
		.WithBlendState(vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha)
		.WithDepthState(vk::CompareOp::eGreaterOrEqual, true, true, false)
		.WithColourFormats({ ColourFormat::BGRA8_sRGB })
		.WithDepthFormat(vk::Format::eD32Sfloat)
		.WithPass(dynamic_cast<VulkanRenderPass*>(app->m_pxRenderToTexturePass)->m_xRenderPass)
		.WithPushConstant(vk::ShaderStageFlagBits::eAll, 0,
			sizeof(glm::mat4) + //modelmat
			sizeof(glm::vec3) + //overrideNormal
			sizeof(uint32_t) +  //useBumpMap
			sizeof(uint32_t) +	//usePhongTess
			sizeof(float) +	//phongTessFactor
			sizeof(uint32_t)	//tessLevel
		);

	VulkanDescriptorSetLayoutBuilder xDescBuilder0 = VulkanDescriptorSetLayoutBuilder().WithBindlessAccess()
		.WithUniformBuffers(1)//camera
		.WithUniformBuffers(1)//lights
		.WithUniformBuffers(1);//misc

	VulkanDescriptorSetLayoutBuilder xDescBuilder1 = VulkanDescriptorSetLayoutBuilder().WithBindlessAccess()
		.WithSamplers(1)//diffuse
		.WithSamplers(1)//normal
		.WithSamplers(1)//roughness
		.WithSamplers(1)//metallic
		.WithSamplers(1);//height

	VulkanDescriptorSetLayoutBuilder xDescBuilder2 = VulkanDescriptorSetLayoutBuilder().WithBindlessAccess()
		.WithUniformBuffers(1);//bones

	vk::DescriptorSetLayout xLayout0 = xDescBuilder0.Build(m_device);
	vk::DescriptorSetLayout xLayout1 = xDescBuilder1.Build(m_device);
	vk::DescriptorSetLayout xLayout2 = xDescBuilder2.Build(m_device);

	xPipelineBuilder = xPipelineBuilder.WithDescriptorSetLayout(0, xLayout0);
	xPipelineBuilder = xPipelineBuilder.WithDescriptorSetLayout(VCE_MATERIAL_TEXTURE_DESC_SET, xLayout1);
	xPipelineBuilder = xPipelineBuilder.WithDescriptorSetLayout(VCE_SKINNING_DESC_SET, xLayout2);
#pragma endregion

	Application::GetInstance()->renderInitialised = true;
}

#ifdef VCE_DEFERRED_SHADING
void VulkanRenderer::SetupDeferredShading() {
	for (uint32_t i = 0; i < m_swapChainImages.size(); i++) {
		m_apxGBufferDiffuse.emplace_back(VulkanTexture2D::CreateColourAttachment(m_width, m_height, 1, vk::Format::eB8G8R8A8Unorm));
		m_apxGBufferNormals.emplace_back(VulkanTexture2D::CreateColourAttachment(m_width, m_height, 1, vk::Format::eB8G8R8A8Unorm));
		m_apxGBufferMaterial.emplace_back(VulkanTexture2D::CreateColourAttachment(m_width, m_height, 1, vk::Format::eB8G8R8A8Unorm));
		m_apxGBufferDepth.emplace_back(VulkanTexture2D::CreateDepthAttachment(m_width, m_height));
		m_apxDeferredDiffuse.emplace_back(VulkanTexture2D::CreateColourAttachment(m_width, m_height, 1, vk::Format::eB8G8R8A8Unorm));
		m_apxDeferredSpecular.emplace_back(VulkanTexture2D::CreateColourAttachment(m_width, m_height, 1, vk::Format::eB8G8R8A8Unorm));
	}
}
#endif

void VulkanRenderer::MainLoop() {
	glfwPollEvents();

	Application* app = Application::GetInstance();

	Scene* scene = app->scene;
	while (true) {
		std::this_thread::yield();
		if (scene->ready)break;//#todo implement mutex here
	}
	app->sceneMutex.lock();


	BeginScene(scene);

	

	DrawFrame(app->scene);

	

	app->sceneMutex.unlock();
}

void VulkanRenderer::CopyToFramebuffer() {
	Application* app = Application::GetInstance();

#ifdef VCE_USE_EDITOR
	app->_pImGuiLayer->Begin();
	app->_pImGuiLayer->OnImGuiRender();
#endif
	



#pragma region gbuffer
	
#ifdef VCE_DEFERRED_SHADING
	m_pxCopyToFramebufferCommandBuffer->SubmitTargetSetup(RendererAPI::s_xGBufferTargetSetup);

	//BeginGBufferRenderPass(commandBuffer, imageIndex);
	VulkanPipeline* pxGBufferPipeline = m_xPipelines.at("GBuffer");

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pxGBufferPipeline->m_xPipeline);

	for (Mesh* mesh : scene->m_axPipelineMeshes.at(pxGBufferPipeline->m_strName)) {
		VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(mesh);
		pxVulkanMesh->BindToCmdBuffer(commandBuffer);

		std::vector<vk::DescriptorSet> axSets;
		for (const vk::DescriptorSet set : pxGBufferPipeline->m_axBufferDescSets)
			axSets.push_back(set);
		if (pxVulkanMesh->GetTexture() != nullptr)
			axSets.push_back(pxVulkanMesh->m_xTexDescSet);

		pxGBufferPipeline->BindDescriptorSets(commandBuffer, axSets, vk::PipelineBindPoint::eGraphics, 0);

		if (pxGBufferPipeline->bUsePushConstants) {
			commandBuffer.pushConstants(pxGBufferPipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, 0, sizeof(glm::mat4), (void*)&mesh->m_xTransform);

			commandBuffer.pushConstants(pxGBufferPipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(glm::mat4), sizeof(glm::vec3), (void*)&m_xOverrideNormal);
			int uValue = app->_pRenderer->m_bUseBumpMaps ? 1 : 0;

			commandBuffer.pushConstants(pxGBufferPipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(glm::mat4) + sizeof(glm::vec3), sizeof(uint32_t), (void*)&uValue);

			int uValueTess = app->_pRenderer->m_bUsePhongTess ? 1 : 0;

			commandBuffer.pushConstants(pxGBufferPipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(glm::mat4) + sizeof(glm::vec3) + sizeof(uint32_t), sizeof(uint32_t), (void*)&uValueTess);

			commandBuffer.pushConstants(pxGBufferPipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(glm::mat4) + sizeof(glm::vec3) + sizeof(uint32_t) + sizeof(uint32_t), sizeof(float), (void*)&app->_pRenderer->m_fPhongTessFactor);
			commandBuffer.pushConstants(pxGBufferPipeline->m_xPipelineLayout, vk::ShaderStageFlagBits::eAll, sizeof(glm::mat4) + sizeof(glm::vec3) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(float), sizeof(float), (void*)&app->_pRenderer->m_uTessLevel);
		}

		commandBuffer.drawIndexed(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances, 0, 0, 0);
	}

	commandBuffer.endRenderPass();
#endif

#pragma endregion

	m_pxCopyToFramebufferCommandBuffer->BeginRecording();

	m_pxCopyToFramebufferCommandBuffer->SubmitTargetSetup(m_xTargetSetups.at("CopyToFramebuffer"), true);

	m_pxCopyToFramebufferCommandBuffer->SetPipeline(m_xPipelines.at("CopyToFramebuffer"));

	VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(app->m_pxQuadMesh);
	m_pxCopyToFramebufferCommandBuffer->SetVertexBuffer(pxVulkanMesh->m_pxVertexBuffer);
	m_pxCopyToFramebufferCommandBuffer->SetIndexBuffer(pxVulkanMesh->m_pxIndexBuffer);

	m_pxCopyToFramebufferCommandBuffer->BindTexture(m_apxEditorSceneTexs[m_currentFrame], 0, 0);
	
	m_pxCopyToFramebufferCommandBuffer->Draw(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances, 0, 0, 0);
	m_pxCopyToFramebufferCommandBuffer->GetCurrentCmdBuffer().endRenderPass();
	
#ifdef VCE_USE_EDITOR

	UpdateImageDescriptor(m_axFramebufferTexDescSet[m_currentFrame], 0, 0, m_apxEditorSceneTexs[m_currentFrame]->m_xImageView, m_xDefaultSampler, vk::ImageLayout::eShaderReadOnlyOptimal);

	BeginImguiRenderPass(m_pxCopyToFramebufferCommandBuffer->GetCurrentCmdBuffer(), m_currentFrame);
	
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_pxCopyToFramebufferCommandBuffer->GetCurrentCmdBuffer());
	app->_pImGuiLayer->End();
	m_pxCopyToFramebufferCommandBuffer->GetCurrentCmdBuffer().endRenderPass();
#endif

	
	m_pxCopyToFramebufferCommandBuffer->EndRecording();
	
}

void VulkanRenderer::DrawSkybox() {
	Application* app = Application::GetInstance();

	m_pxSkyboxCommandBuffer->BeginRecording();

	m_pxSkyboxCommandBuffer->SubmitTargetSetup(m_xTargetSetups.at("RenderToTexture"), false);

	VulkanPipeline* pxSkyboxPipeline = m_xPipelines.at("Skybox");
	m_pxSkyboxCommandBuffer->SetPipeline(&pxSkyboxPipeline->m_xPipeline);

	VulkanManagedUniformBuffer* pxCamUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pCameraUBO);
	m_pxSkyboxCommandBuffer->BindBuffer(pxCamUBO->ppBuffers[m_currentFrame], 0, 0);

	VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(app->m_pxQuadMesh);
	m_pxSkyboxCommandBuffer->SetVertexBuffer(pxVulkanMesh->m_pxVertexBuffer);
	m_pxSkyboxCommandBuffer->SetIndexBuffer(pxVulkanMesh->m_pxIndexBuffer);
	m_pxSkyboxCommandBuffer->Draw(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances, 0, 0, 0);

	m_pxSkyboxCommandBuffer->GetCurrentCmdBuffer().endRenderPass();

	m_pxSkyboxCommandBuffer->EndRecording();
}

void VulkanRenderer::DrawOpaqueMeshes() {
	Application* app = Application::GetInstance();

	m_pxOpaqueMeshesCommandBuffer->BeginRecording();

	m_pxOpaqueMeshesCommandBuffer->SubmitTargetSetup(m_xTargetSetups.at("RenderToTexture"), true);


	m_pxOpaqueMeshesCommandBuffer->SetPipeline(&m_xPipelines.at("Meshes")->m_xPipeline);

	Application::MeshRenderData xMeshRenderData;

	xMeshRenderData.xOverrideNormal = m_xOverrideNormal;
	xMeshRenderData.uUseBumpMap = app->_pRenderer->m_bUseBumpMaps ? 1 : 0;
	xMeshRenderData.uUsePhongTess = app->_pRenderer->m_bUsePhongTess ? 1 : 0;
	xMeshRenderData.fPhongTessFactor = app->_pRenderer->m_fPhongTessFactor;
	xMeshRenderData.uTessLevel = app->_pRenderer->m_uTessLevel;

	app->m_pxMiscMeshRenderDataUBO->UploadData(&xMeshRenderData, sizeof(Application::MeshRenderData), m_currentFrame, 0);

	VulkanManagedUniformBuffer* pxCamUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pCameraUBO);
	m_pxOpaqueMeshesCommandBuffer->BindBuffer(pxCamUBO->ppBuffers[m_currentFrame], 0, 0);

	VulkanManagedUniformBuffer* pxLightUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pLightUBO);
	m_pxOpaqueMeshesCommandBuffer->BindBuffer(pxLightUBO->ppBuffers[m_currentFrame], 1, 0);


	VulkanManagedUniformBuffer* pxMiscMeshRenderDataUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->m_pxMiscMeshRenderDataUBO);
	m_pxOpaqueMeshesCommandBuffer->BindBuffer(pxMiscMeshRenderDataUBO->ppBuffers[m_currentFrame], 2, 0);


	for (Mesh* mesh : app->scene->m_axPipelineMeshes.at("Meshes")) {
		VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(mesh);
		m_pxOpaqueMeshesCommandBuffer->SetVertexBuffer(pxVulkanMesh->m_pxVertexBuffer);
		m_pxOpaqueMeshesCommandBuffer->SetIndexBuffer(pxVulkanMesh->m_pxIndexBuffer);

		m_pxOpaqueMeshesCommandBuffer->BindMaterial(mesh->m_pxMaterial,1);


		m_pxOpaqueMeshesCommandBuffer->PushConstant(&mesh->m_xTransform._matrix, sizeof(glm::mat4));

		m_pxOpaqueMeshesCommandBuffer->Draw(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances);
	}

	m_pxOpaqueMeshesCommandBuffer->GetCurrentCmdBuffer().endRenderPass();

	m_pxOpaqueMeshesCommandBuffer->EndRecording();
}

void VulkanRenderer::DrawSkinnedMeshes() {
	Application* app = Application::GetInstance();

	m_pxSkinnedMeshesCommandBuffer->BeginRecording();

	m_pxSkinnedMeshesCommandBuffer->SubmitTargetSetup(m_xTargetSetups.at("RenderToTexture"), false);


	m_pxSkinnedMeshesCommandBuffer->SetPipeline(&m_xPipelines.at("SkinnedMeshes")->m_xPipeline);

	Application::MeshRenderData xMeshRenderData;

	xMeshRenderData.xOverrideNormal = m_xOverrideNormal;
	xMeshRenderData.uUseBumpMap = app->_pRenderer->m_bUseBumpMaps ? 1 : 0;
	xMeshRenderData.uUsePhongTess = app->_pRenderer->m_bUsePhongTess ? 1 : 0;
	xMeshRenderData.fPhongTessFactor = app->_pRenderer->m_fPhongTessFactor;
	xMeshRenderData.uTessLevel = app->_pRenderer->m_uTessLevel;

	app->m_pxMiscMeshRenderDataUBO->UploadData(&xMeshRenderData, sizeof(Application::MeshRenderData), m_currentFrame, 0);


	VulkanManagedUniformBuffer* pxCamUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pCameraUBO);
	m_pxSkinnedMeshesCommandBuffer->BindBuffer(pxCamUBO->ppBuffers[m_currentFrame], 0, 0);

	VulkanManagedUniformBuffer* pxLightUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->_pLightUBO);
	m_pxSkinnedMeshesCommandBuffer->BindBuffer(pxLightUBO->ppBuffers[m_currentFrame], 1, 0);




	VulkanManagedUniformBuffer* pxMiscMeshRenderDataUBO = dynamic_cast<VulkanManagedUniformBuffer*>(app->m_pxMiscMeshRenderDataUBO);
	m_pxSkinnedMeshesCommandBuffer->BindBuffer(pxMiscMeshRenderDataUBO->ppBuffers[m_currentFrame], 2, 0);

	for (Mesh* mesh : app->scene->m_axPipelineMeshes.at("SkinnedMeshes")) {
		VulkanMesh* pxVulkanMesh = dynamic_cast<VulkanMesh*>(mesh);
		m_pxSkinnedMeshesCommandBuffer->SetVertexBuffer(pxVulkanMesh->m_pxVertexBuffer);
		m_pxSkinnedMeshesCommandBuffer->SetIndexBuffer(pxVulkanMesh->m_pxIndexBuffer);

		pxVulkanMesh->m_pxBoneBuffer->UploadData(pxVulkanMesh->m_xBoneMats.data(), pxVulkanMesh->m_xBoneMats.size() * sizeof(glm::mat4), m_currentFrame);

		m_pxSkinnedMeshesCommandBuffer->BindMaterial(mesh->m_pxMaterial,1);

		m_pxSkinnedMeshesCommandBuffer->BindAnimation(mesh,2);

		RendererAPI::MeshPushConstantData xPushConstants;
		xPushConstants.m_xModelMat = mesh->m_xTransform._matrix;
		xPushConstants.m_uAnimate = bAnimate ? 1 : 0;
		xPushConstants.m_fAlpha = fAnimAlpha;

		m_pxSkinnedMeshesCommandBuffer->PushConstant(&xPushConstants, sizeof(RendererAPI::MeshPushConstantData));


		m_pxSkinnedMeshesCommandBuffer->Draw(pxVulkanMesh->m_uNumIndices, pxVulkanMesh->m_uNumInstances);
	}

	m_pxSkinnedMeshesCommandBuffer->GetCurrentCmdBuffer().endRenderPass();

	m_pxSkinnedMeshesCommandBuffer->EndRecording();
}

void VulkanRenderer::DrawFrame(Scene* scene) {
	uint32_t iImageIndex = AcquireSwapchainImage();
	if (iImageIndex == -1) {
		RecreateSwapChain();
		return;
	}

	DrawOpaqueMeshes();

	DrawSkinnedMeshes();

	DrawSkybox();

	

	

	CopyToFramebuffer();
	
	m_pxRendererAPI->Platform_SubmitCmdBuffers();
	

	Present(iImageIndex, &m_renderFinishedSemaphores[m_currentFrame], 1);


	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VeryCoolEngine::VulkanRenderer::BeginScene(Scene* scene)
{
	Application* app = Application::GetInstance();

	RendererAPI::FrameConstants xFrameConstants;
	xFrameConstants.m_xViewMat = scene->camera->BuildViewMatrix();
	xFrameConstants.m_xProjMat = scene->camera->BuildProjectionMatrix();
	xFrameConstants.m_xViewProjMat = xFrameConstants.m_xProjMat * xFrameConstants.m_xViewMat;
	glm::vec3 tempCamPos = scene->camera->GetPosition();
	xFrameConstants.m_xCamPos = { tempCamPos.x, tempCamPos.y, tempCamPos.z,0 };
	app->_pCameraUBO->UploadData(&xFrameConstants, sizeof(RendererAPI::FrameConstants), m_currentFrame, 0);


	RendererAPI::LightData xLightData;
	memcpy(xLightData.GetLightsPtr(), scene->lights.data(), sizeof(RendererAPI::Light) * scene->lights.size());
	xLightData.CalculateNumLights();

	app->_pLightUBO->UploadData(&xLightData, xLightData.GetUploadSize(), m_currentFrame, 0);

}

void VeryCoolEngine::VulkanRenderer::RenderThreadFunction()
{
	Application* app = Application::GetInstance();
#ifdef VCE_USE_EDITOR
	app->_pImGuiLayer = new ImGuiLayer();
	app->PushOverlay(app->_pImGuiLayer);
#endif
	while (app->_running) {
		MainLoop();
	}
}

void RendererAPI::Platform_SubmitCmdBuffers() {
	VulkanRenderer* pxRenderer = VulkanRenderer::GetInstance();

	vk::PipelineStageFlags eWaitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	std::vector<vk::CommandBuffer> xPlatformCmdBufs;
	for (void* pCmdBuf : s_xCmdBuffersToSubmit) {
		vk::CommandBuffer& xBuf = *reinterpret_cast<vk::CommandBuffer*>(pCmdBuf);
		xPlatformCmdBufs.push_back(xBuf);
	}

	vk::SubmitInfo xSubmitInfo = vk::SubmitInfo()
		.setCommandBufferCount(s_xCmdBuffersToSubmit.size())
		.setPCommandBuffers(xPlatformCmdBufs.data())
		.setPWaitSemaphores(&pxRenderer->GetCurrentImageAvailableSem())
		.setPSignalSemaphores(&pxRenderer->GetCurrentRenderCompleteSem())
		.setWaitSemaphoreCount(1)
		.setSignalSemaphoreCount(1)
		.setWaitDstStageMask(eWaitStages);

	pxRenderer->GetGraphicsQueue().submit(xSubmitInfo, pxRenderer->GetCurrentInFlightFence());

	//TODO: put this in end frame when I eventually write it
	s_xCmdBuffersToSubmit.clear();
}