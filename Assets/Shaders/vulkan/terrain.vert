#version 450 core

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec2 _aUV;
layout(location = 2) in vec3 _aNormal;
layout(location = 3) in vec4 _aTangent;


layout(std140, binding=0) uniform matrices{
	mat4 _uViewMat;
	mat4 _uProjMat;
	mat4 _uViewProjMat;
	vec4 _uCamPos;//4 bytes of padding
};

layout(location = 0) out vec2 UV;
layout(location = 1) out vec3 Normal;
layout(location = 2) out vec3 WorldPos;
layout(location = 3) out vec3 Tangent;
layout(location = 4) out vec3 Binormal;

void main(){
	WorldPos = _aPosition;//#todo model mat
	UV = _aUV;
	mat3 normalMat = mat3(1);//#todo transpose(inverse(mat3(modelMatrix)));
	Normal = normalize(normalMat * normalize(_aNormal));
	vec3 wNormal = normalize(normalMat * normalize(_aNormal));
	vec3 wTangent = normalize(normalMat * normalize(_aTangent.xyz));
	Tangent = wTangent;
	Binormal = cross(wTangent,wNormal) * _aTangent.w;

	//#TODO delete
	Normal = _aNormal;

	gl_Position = _uViewProjMat * vec4(_aPosition,1);//#todo model mat
}