#version 450 core

#include "Terrain_Common.h"

layout(location = 0) in vec3 a_xPosition;
layout(location = 1) in vec2 a_xUV;
layout(location = 2) in vec3 a_xNormal;
layout(location = 3) in vec3 a_xTangent;
layout(location = 4) in vec3 a_xBitangent;

#ifdef USE_TESSELATION
layout(location = 0) out vec2 o_xUV;
layout(location = 1) out vec3 o_xNormal;
layout(location = 2) out vec3 o_xTangent;
layout(location = 3) out vec3 o_xBitangent;
layout(location = 4) out vec3 o_xWorldPos;
#else
layout(location = 0) out vec2 o_xUV;
layout(location = 1) out vec3 o_xNormal;
layout(location = 2) out vec3 o_xWorldPos;
layout(location = 3) out vec3 o_xTangent;
layout(location = 4) out vec3 o_xBitangent;
#endif

vec3 ComputeNormal(vec3 p0, vec3 p1, vec3 p2) {
    vec3 edge1 = p1 - p0;
    vec3 edge2 = p2 - p0;
    return normalize(cross(edge1, edge2));
}

void main(){
	o_xUV = a_xUV;
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	o_xNormal = normalize(normalMatrix * normalize(a_xNormal));
	o_xTangent = normalize(normalMatrix * normalize(a_xTangent));
	o_xBitangent = cross(o_xTangent, o_xNormal);
	o_xWorldPos = (modelMatrix * vec4(a_xPosition,1)).xyz;
	

	#ifdef USE_TESSELATION
	gl_Position = modelMatrix * vec4(a_xPosition,1);
	#else
	
	
	float height = texture(heightMap , o_xUV ).x;
    o_xWorldPos.y += height * heightMultiplier;
    o_xWorldPos.y += texture(detailHeightmap , o_xUV * uvScale ).x * (heightMultiplier * 0.001f);
	
	
	
	gl_Position = _uViewProjMat * vec4(o_xWorldPos,1);
	#endif
}