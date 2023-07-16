#include "vcepch.h"
#include "VertexArray.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace VeryCoolEngine {
	
	VertexArray* VertexArray::Create() {
#ifdef VCE_OPENGL
		return new OpenGLVertexArray();
#endif
	}
}