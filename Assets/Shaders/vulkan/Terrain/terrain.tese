#version 450 core

#include "Terrain_Common.h"

layout(triangles , ccw) in;


layout(location = 0) in vec2 _aUV[];
layout(location = 1) in vec3 _aNormal[];
layout(location = 2) in vec3 _aTangent[];
layout(location = 3) in vec3 _aBitangent[];
layout(location = 4) in vec3 _aWorldPos[];
layout(location = 5) in vec2 o_xOtherUV0[];
layout(location = 6) in vec2 o_xOtherUV1[];
layout(location = 7) in vec3 o_xOtherPos0[];
layout(location = 8) in vec3 o_xOtherPos1[];

layout(location = 0) out vec2 _oUV;
layout(location = 1) out vec3 _oNormal;
layout(location = 2) out vec3 _oWorldPos;
layout(location = 3) out mat3 _oTBN;


 
vec3 TriMixVec3(vec3 a, vec3 b, vec3 c) {
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	vec3 val0 = a;
	vec3 val1 = b;
	vec3 val2 = c;

	vec3 val = u * val0 + v * val1 + w * val2;
	return val;
}

vec2 TriMixVec2(vec2 a, vec2 b, vec2 c) {
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	vec2 val0 = a;
	vec2 val1 = b;
	vec2 val2 = c;

	vec2 val = u * val0 + v * val1 + w * val2;
	return val;
}

void main(){
	vec3 combinedPos = TriMixVec3(_aWorldPos[0], _aWorldPos[1], _aWorldPos[2]);

    _oUV = TriMixVec2(_aUV[0], _aUV[1], _aUV[2]);
	
	vec2 otherUV0 = TriMixVec2(o_xOtherUV0[0], o_xOtherUV0[1], o_xOtherUV0[2]);
	vec2 otherUV1 = TriMixVec2(o_xOtherUV1[0], o_xOtherUV1[1], o_xOtherUV1[2]);
	vec3 otherPos0 = TriMixVec3(o_xOtherPos0[0], o_xOtherPos0[1], o_xOtherPos0[2]);
	vec3 otherPos1 = TriMixVec3(o_xOtherPos1[0], o_xOtherPos1[1], o_xOtherPos1[2]);

    _oNormal = TriMixVec3(_aNormal[0], _aNormal[1], _aNormal[2]);
    _oWorldPos = combinedPos;

    vec3 combinedTangent = TriMixVec3(_aTangent[0], _aTangent[1], _aTangent[2]);
    vec3 combinedBitangent = TriMixVec3(_aBitangent[0], _aBitangent[1], _aBitangent[2]);

    _oTBN = mat3(normalize(combinedTangent), normalize(combinedBitangent), normalize(_oNormal));

    gl_Position = _uViewProjMat * vec4(_oWorldPos, 1);
	
}