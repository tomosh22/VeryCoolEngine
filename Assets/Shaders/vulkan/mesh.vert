#version 450 core

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec2 _aUV;
layout(location = 2) in vec3 _aNormal;
layout(location = 3) in vec3 _aTangent;
layout(location = 4) in vec3 _aBitangent;

layout(push_constant) uniform ModelMatrix{
	mat4 modelMatrix;
};

layout(std140, set = 0, binding = 2) uniform Misc{
	vec3 overrideNormal;
	int useBumpMap;
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

	_oUV = _aUV * 5;
	_oNormal = _aNormal;
	_oTangent = _aTangent;
	_oBitangent = _aBitangent;
	_oWorldPos = (modelMatrix * vec4(_aPosition,1)).xyz;

	gl_Position = vec4(_aPosition,1);
}