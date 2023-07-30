#version 450 core

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec2 _aUV;
layout(location = 2) in vec3 _aNormal;


layout(std140, binding=0) uniform matrices{
	mat4 _uViewMat;
	mat4 _uProjMat;
	mat4 _uViewProjMat;
	vec4 _uCamPos;//4 bytes of padding
};

out vec2 UV;
out vec3 Normal;
out vec3 WorldPos;

void main(){
	WorldPos = _aPosition;//#todo model mat
	UV = _aUV;
	Normal = _aNormal;//#todo transpose model mat
	gl_Position = _uViewProjMat * vec4(_aPosition,1);//#todo model mat
}