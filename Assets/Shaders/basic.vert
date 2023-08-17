#version 450 core

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec2 _aUV;
layout(location = 2) in vec3 _aNormal;
layout(location = 3) in vec4 _aTangent;

uniform mat4 _uModelMat;

layout(std140, binding=0) uniform matrices{
	mat4 _uViewMat;
	mat4 _uProjMat;
	mat4 _uViewProjMat;
	vec4 _uCamPos;//vec3 plus 4 bytes of padding
};

out vec2 UV;
out vec3 Normal;
out vec3 WorldPos;
out vec3 Tangent;
out vec3 Binormal;

void main(){
	WorldPos = (_uModelMat * vec4(_aPosition,1)).xyz;
	UV = _aUV;
	mat3 normalMat = mat3(1);//#todo transpose(inverse(mat3(modelMatrix)));
	Normal = normalize(normalMat * normalize(_aNormal));
	vec3 wNormal = normalize(normalMat * normalize(_aNormal));
	vec3 wTangent = normalize(normalMat * normalize(_aTangent.xyz));
	Tangent = wTangent;
	Binormal = cross(wTangent,wNormal) * _aTangent.w;



	gl_Position = _uViewProjMat * vec4(WorldPos,1);//#todo model mat
}