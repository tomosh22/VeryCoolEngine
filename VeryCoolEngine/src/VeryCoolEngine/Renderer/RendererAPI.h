#pragma once

static constexpr const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

namespace VeryCoolEngine {
	class VertexBuffer;
	class IndexBuffer;
	class ManagedUniformBuffer;
	class Material;
	class RendererAPI
	{
	public:
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
			virtual void BindTexture(void* pxTexture, uint32_t uBindPoint) = 0;
			virtual void BindBuffer(void* pxBuffer, uint32_t uBindPoint) = 0;
			virtual void PushConstant(void* pData, size_t uSize) = 0;
			virtual void UploadUniformData(void* pData, size_t uSize) = 0;

			virtual void BindMaterial(Material* pxMaterial) = 0;

			virtual void* Platform_GetCurrentCmdBuffer() const = 0;

			ManagedUniformBuffer* m_pxUniformBuffer;
		};

		std::vector<void*> s_xCmdBuffersToSubmit;
		void Platform_SubmitCmdBuffers();

		//static TargetSetup s_xGBufferTargetSetup;
	};
}


