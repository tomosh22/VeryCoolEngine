#version 450 core

#include "Mesh_NoSkin_Common.h"

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec2 _aUV;
layout(location = 2) in vec3 _aNormal;
layout(location = 3) in vec3 _aTangent;
layout(location = 4) in vec3 _aBitangent;

layout(std140, set = 0, binding = 2) uniform Misc{
	vec3 overrideNormal;
	int useBumpMap;
	int visualiseNormals;
	int usePhongTess;
	float phongTessFactor;
	int tessLevel;
};

layout(location = 0) out vec2 _oUV;
layout(location = 1) out vec3 _oNormal;
layout(location = 2) out vec3 _oTangent;
layout(location = 3) out vec3 _oBitangent;
layout(location = 4) out vec3 _oWorldPos;

void main(){

	_oUV = _aUV;
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	_oNormal = normalize(normalMatrix * normalize(_aNormal));
	_oTangent = normalize(normalMatrix * normalize(_aTangent));
	_oBitangent = cross(_oTangent, _oNormal);
	_oWorldPos = (modelMatrix * vec4(_aPosition,1)).xyz;

	gl_Position = vec4(_aPosition,1);
}