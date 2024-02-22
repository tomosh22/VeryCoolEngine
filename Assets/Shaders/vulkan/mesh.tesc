#version 450 core

#include "Mesh_NoSkin_Common.h"

layout(vertices = 3) out;


layout(location = 0) in vec2 _aUV[];
layout(location = 1) in vec3 _aNormal[];
layout(location = 2) in vec3 _aTangent[];
layout(location = 3) in vec3 _aBitangent[];
layout(location = 4) in vec3 _aWorldPos[];

layout(location = 0) out vec2 _oUV[];
layout(location = 1) out vec3 _oNormal[];
layout(location = 2) out vec3 _oTangent[];
layout(location = 3) out vec3 _oBitangent[];
layout(location = 4) out vec3 _oWorldPos[];

layout(std140, set = 0, binding = 2) uniform Misc{
	vec3 overrideNormal;
	int useBumpMap;
	int visualiseNormals;
	int usePhongTess;
	float phongTessFactor;
	int tessLevel;
};

void main(){

	gl_TessLevelInner [0] = tessLevel;
	gl_TessLevelInner [1] = tessLevel;
	gl_TessLevelOuter [0] = tessLevel;
	gl_TessLevelOuter [1] = tessLevel;
	gl_TessLevelOuter [2] = tessLevel;
	gl_TessLevelOuter [3] = tessLevel;

	_oUV[gl_InvocationID] = _aUV[gl_InvocationID];
	_oNormal[gl_InvocationID] = _aNormal[gl_InvocationID];
	_oTangent[gl_InvocationID] = _aTangent[gl_InvocationID];
	_oBitangent[gl_InvocationID] = _aBitangent[gl_InvocationID];
	_oWorldPos[gl_InvocationID] = _aWorldPos[gl_InvocationID];
}