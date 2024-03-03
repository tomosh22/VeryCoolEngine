#version 450 core

#include "Terrain_Common.h"

layout(location = 0) in vec3 a_xPosition;
layout(location = 1) in vec2 a_xUV;
layout(location = 2) in vec3 a_xNormal;
layout(location = 3) in vec3 a_xTangent;
layout(location = 4) in vec3 a_xBitangent;


layout(location = 0) out vec2 o_xUV;
layout(location = 1) out vec3 o_xNormal;
layout(location = 2) out vec3 o_xTangent;
layout(location = 3) out vec3 o_xBitangent;
layout(location = 4) out vec3 o_xWorldPos;

void main(){
	o_xUV = a_xUV;
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	o_xNormal = normalize(normalMatrix * normalize(a_xNormal));
	o_xTangent = normalize(normalMatrix * normalize(a_xTangent));
	o_xBitangent = cross(o_xTangent, o_xNormal);
	o_xWorldPos = (modelMatrix * vec4(a_xPosition,1)).xyz;

	gl_Position = vec4(a_xPosition,1);
}