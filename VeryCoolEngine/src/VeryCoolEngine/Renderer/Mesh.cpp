#include "vcepch.h"
#include "Mesh.h"
#include "Platform/OpenGL/OpenGLMesh.h"

namespace VeryCoolEngine {

	Mesh* Mesh::Create() {
#ifdef VCE_OPENGL
		return new OpenGLMesh();
#endif
	}
}