#include "vcepch.h"
#include "VertexArray.h"

namespace VeryCoolEngine {
	
	VertexArray* VertexArray::Create() {
#ifdef VCE_OPENGL
		return new OpenGLVertexArray();
#endif
		VCE_INFO("implement me");
		return nullptr;
	}
}