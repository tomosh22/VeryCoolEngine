#version 450 core

#include "Terrain_Common.h"

layout(vertices = 3) out;


layout(location = 0) in vec2 a_xUV[];
layout(location = 1) in vec3 a_xNormal[];
layout(location = 2) in vec3 a_xTangent[];
layout(location = 3) in vec3 a_xBitangent[];
layout(location = 4) in vec3 a_xWorldPos[];

layout(location = 0) out vec2 o_xUV[];
layout(location = 1) out vec3 o_xNormal[];
layout(location = 2) out vec3 o_xTangent[];
layout(location = 3) out vec3 o_xBitangent[];
layout(location = 4) out vec3 o_xWorldPos[];
layout(location = 5) out vec2 o_xOtherUV0[];
layout(location = 6) out vec2 o_xOtherUV1[];
layout(location = 7) out vec3 o_xOtherPos0[];
layout(location = 8) out vec3 o_xOtherPos1[];


void main(){

	gl_TessLevelInner [0] = tessLevel;
	gl_TessLevelInner [1] = tessLevel;
	gl_TessLevelOuter [0] = tessLevel;
	gl_TessLevelOuter [1] = tessLevel;
	gl_TessLevelOuter [2] = tessLevel;
	gl_TessLevelOuter [3] = tessLevel;

	o_xUV[gl_InvocationID] = a_xUV[gl_InvocationID];
	o_xNormal[gl_InvocationID] = a_xNormal[gl_InvocationID];
	o_xTangent[gl_InvocationID] = a_xTangent[gl_InvocationID];
	o_xBitangent[gl_InvocationID] = a_xBitangent[gl_InvocationID];
	o_xWorldPos[gl_InvocationID] = a_xWorldPos[gl_InvocationID];
	
	if(gl_InvocationID == 0){
		o_xOtherUV0[gl_InvocationID] = o_xUV[1];
		o_xOtherUV1[gl_InvocationID] = o_xUV[2];
		o_xOtherPos0[gl_InvocationID] = o_xWorldPos[1];
		o_xOtherPos1[gl_InvocationID] = o_xWorldPos[2];
	}
	else if(gl_InvocationID == 1){
		o_xOtherUV0[gl_InvocationID] = o_xUV[2];
		o_xOtherUV1[gl_InvocationID] = o_xUV[0];
		o_xOtherPos0[gl_InvocationID] = o_xWorldPos[2];
		o_xOtherPos1[gl_InvocationID] = o_xWorldPos[0];
	}
	else if(gl_InvocationID == 2){
		o_xOtherUV0[gl_InvocationID] = o_xUV[0];
		o_xOtherUV1[gl_InvocationID] = o_xUV[1];
		o_xOtherPos0[gl_InvocationID] = o_xWorldPos[0];
		o_xOtherPos1[gl_InvocationID] = o_xWorldPos[1];
	}
}