#pragma once

#include <glm/glm.hpp>
static constexpr const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

namespace VeryCoolEngine {
	class VertexBuffer;
	class IndexBuffer;
	class ManagedUniformBuffer;
	class Material;
	class Mesh;
	class RendererAPI
	{
	public:
		constexpr static uint32_t g_uMaxLights = 100;

		struct FrameConstants {
			glm::mat4 m_xViewMat;
			glm::mat4 m_xProjMat;
			glm::mat4 m_xViewProjMat;
			glm::vec4 m_xCamPos;//4 bytes of padding
		};

		struct Light {
			float x;
			float y;
			float z;
			float radius = 0;
			float r;
			float g;
			float b;
			float a;
		};

		//TODO: don't like that I'm creating g_uMaxLights lights every time I create one of these
		struct LightData {
		public:
			Light* GetLightsPtr() { return &m_axLights[0]; }
			uint32_t GetNumLights() {
				if (!m_bCalculatedNumLights) CalculateNumLights();
				return m_uNumLights;
			}
			
			size_t GetUploadSize() {
				if (!m_bCalculatedNumLights) CalculateNumLights();
				return sizeof(m_uNumLights) + sizeof(m_Pad) + sizeof(Light) * m_uNumLights;
			}
			void CalculateNumLights() {
				m_uNumLights = 0;
				for (uint32_t i = 0; i < g_uMaxLights; i++)
					if (m_axLights[i].radius != 0)
						m_uNumLights++;
					else
						break;
				m_bCalculatedNumLights = true;
			}
		private:
			uint32_t m_uNumLights;
			uint32_t m_Pad[3];
			Light m_axLights[g_uMaxLights];
			bool m_bCalculatedNumLights = false;
		};
		struct MeshPushConstantData {
			glm::mat4 m_xModelMat;
			uint32_t m_uAnimate;
			float m_fAlpha;
		};
		struct RenderTarget {
			enum class Format : uint32_t {
				None,
				//colour
				B8G8R8A8Unorm,
				B8G8R8A8Srgb,

				//depth/stencil
				D32Sfloat
			};
			enum class LoadAction : uint32_t {
				DontCare, Load, Clear
			};
			enum class StoreAction : uint32_t {
				DontCare, Store
			};
			enum class Usage : uint32_t {
				RenderTarget, ShaderRead, Present
			};
			Format m_eFormat = Format::None;
			LoadAction m_eLoadAction = LoadAction::DontCare;
			StoreAction m_eStoreAction = StoreAction::DontCare;
			Usage m_eUsage = Usage::RenderTarget;
			uint32_t m_uWidth = 0;
			uint32_t m_uHeight = 0;
			void* m_pPlatformImageView;
		};
		struct TargetSetup {
			std::vector<RenderTarget> m_xColourAttachments;
			RenderTarget m_xDepthStencil;
			std::string m_strName;
		};

		class CommandBuffer {
		public:
			CommandBuffer() = default;
			virtual void BeginRecording() = 0;
			virtual void EndRecording(bool bSubmit = true) = 0;
			virtual void SetVertexBuffer(VertexBuffer* xVertexBuffer, uint32_t uBindPoint = 0) = 0;
			virtual void SetIndexBuffer(IndexBuffer* xIndexBuffer) = 0;
			virtual void Draw(uint32_t uNumIndices, uint32_t uNumInstances = 1, uint32_t uVertexOffset = 0, uint32_t uIndexOffset = 0, uint32_t uInstanceOffset = 0) = 0;
			virtual void SubmitTargetSetup(const TargetSetup& xTargetSetup, bool bClear) = 0;
			virtual void SetPipeline(void* pxPipeline) = 0;
			virtual void BindTexture(void* pxTexture, uint32_t uBindPoint, uint32_t uSet) = 0;
			virtual void BindBuffer(void* pxBuffer, uint32_t uBindPoint, uint32_t uSet) = 0;

			virtual void PushConstant(void* pData, size_t uSize) = 0;

			virtual void UploadUniformData(void* pData, size_t uSize) = 0;

			virtual void BindMaterial(Material* pxMaterial, uint32_t uSet) = 0;
			//TODO: this will be a model when i stop duplicating animation data
			virtual void BindAnimation(Mesh* pxModel, uint32_t uSet) = 0;

			virtual void* Platform_GetCurrentCmdBuffer() const = 0;

			ManagedUniformBuffer* m_pxUniformBuffer;
		};

		std::vector<void*> s_xCmdBuffersToSubmit;
		void Platform_SubmitCmdBuffers();

		//static TargetSetup s_xGBufferTargetSetup;
	};
}


