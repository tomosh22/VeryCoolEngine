#version 450 core

#include "Foliage_Common.h"

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec2 _aUV;
layout(location = 2) in vec3 _aInstancePosition;




layout(location = 0) out vec2 UV;



void main(){

	bool bBillboard = true;
	
	UV = _aUV;
	
	gl_Position = _uViewProjMat * vec4(_aPosition + _aInstancePosition, 1);
}