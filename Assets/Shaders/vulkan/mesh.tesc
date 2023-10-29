#version 450 core

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


layout(push_constant) uniform PushConstantVert{
	mat4 modelMatrix;
	vec3 overrideNormal;
	int useBumpMap;
};

void main(){
	gl_TessLevelInner [0] = 8;
	gl_TessLevelInner [1] = 8;
	gl_TessLevelOuter [0] = 8;
	gl_TessLevelOuter [1] = 8;
	gl_TessLevelOuter [2] = 8;
	gl_TessLevelOuter [3] = 8;

	_oUV[gl_InvocationID] = _aUV[gl_InvocationID];
	_oNormal[gl_InvocationID] = _aNormal[gl_InvocationID];
	_oTangent[gl_InvocationID] = _aTangent[gl_InvocationID];
	_oBitangent[gl_InvocationID] = _aBitangent[gl_InvocationID];
	_oWorldPos[gl_InvocationID] = _aWorldPos[gl_InvocationID];

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}