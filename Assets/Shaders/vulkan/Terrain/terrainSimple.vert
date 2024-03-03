#version 450 core

#include "Terrain_Common.h"

layout(location = 0) in vec3 a_xPosition;
layout(location = 1) in vec2 a_xUV;
layout(location = 2) in vec3 a_xNormal;
layout(location = 3) in vec3 a_xTangent;
layout(location = 4) in vec3 a_xBitangent;


layout(location = 0) out vec2 o_xUV;
layout(location = 1) out vec3 o_xNormal;
layout(location = 2) out vec3 o_xWorldPos;
layout(location = 3) out mat3 o_xTBN;


void main(){
	o_xUV = a_xUV;
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	o_xNormal = normalize(normalMatrix * normalize(a_xNormal));
	o_xWorldPos = (modelMatrix * vec4(a_xPosition,1)).xyz;
	
	o_xTBN = mat3(normalize(a_xTangent), normalize(a_xBitangent), normalize(a_xNormal));

	gl_Position = _uViewProjMat * modelMatrix * vec4(a_xPosition,1);
}