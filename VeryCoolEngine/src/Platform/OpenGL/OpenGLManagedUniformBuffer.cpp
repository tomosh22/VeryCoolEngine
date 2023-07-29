#include "vcepch.h"
#include "OpenGLManagedUniformBuffer.h"
namespace VeryCoolEngine {

	OpenGLManagedUniformBuffer::OpenGLManagedUniformBuffer(uint32_t size, uint8_t numFrames)
	{
		numFrames = 1; //#todo is there ever any need for more than one in opengl?
		_pUBOs = new GLuint[numFrames];
		glGenBuffers(numFrames, _pUBOs);
		for (uint8_t i = 0; i < numFrames; i++)
		{
			GLuint ubo = _pUBOs[i];
			glBindBuffer(GL_UNIFORM_BUFFER, ubo);
			glBindBufferBase(GL_UNIFORM_BUFFER,1, ubo);//#todo stop hardcoding 1 (0 taken by viewProj)
			glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);//#todo check static draw is right
			glBindBufferRange(GL_UNIFORM_BUFFER,1,ubo,0,size);//#todo stop hardcoding 1 again
		}
	}

	void OpenGLManagedUniformBuffer::UploadData(const void* const data, uint32_t size, uint8_t frame, uint32_t offset)
	{
		glBindBuffer(GL_UNIFORM_BUFFER,_pUBOs[0]);//#todo frame index
		glBufferData(GL_UNIFORM_BUFFER,size,data,GL_STATIC_DRAW);//#todo check static draw is ok
	}

}
