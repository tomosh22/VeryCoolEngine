#pragma once
#ifdef VCE_VULKAN
#include <vulkan/vulkan.h>
#endif

static constexpr const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

namespace VeryCoolEngine {
	class VertexBuffer;
	class IndexBuffer;
	namespace RendererAPI
	{
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
			Format m_eFormat = Format::None;
			LoadAction m_eLoadAction = LoadAction::DontCare;
			StoreAction m_eStoreAction = StoreAction::DontCare;
			uint32_t m_uWidth = 0;
			uint32_t m_uHeight = 0;
			void* m_pPlatformImageView;
		};
		struct TargetSetup {
			std::vector<RenderTarget> m_xColourAttachments;
			RenderTarget m_xDepthStencil;
		};

		class CommandBuffer {
		public:
			CommandBuffer() = default;
			virtual void BeginRecording() = 0;
			virtual void EndRecording(bool bSubmit = true) = 0;
			virtual void SetVertexBuffer(VertexBuffer* xVertexBuffer, uint32_t uBindPoint = 0) = 0;
			virtual void SetIndexBuffer(IndexBuffer* xIndexBuffer) = 0;
			virtual void Draw(uint32_t uNumIndices, uint32_t uNumInstances = 1, uint32_t uVertexOffset = 0, uint32_t uIndexOffset = 0, uint32_t uInstanceOffset = 0) = 0;
			virtual void SubmitTargetSetup(const TargetSetup& xTargetSetup) = 0;
			virtual void SetPipeline(void* pxPipeline) = 0;

			virtual void* Platform_GetCurrentCmdBuffer() const = 0;
		};

		static std::vector<void*> s_xCmdBuffersToSubmit;
		static void Platform_SubmitCmdBuffers();

		static TargetSetup s_xGBufferTargetSetup;
	};
}


