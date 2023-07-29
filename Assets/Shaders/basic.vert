#version 450 core

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec2 _aUV;
layout(location = 2) in vec3 _aNormal;


layout(std140, binding=0) uniform matrices{ mat4 _uViewProjMat;};

out vec2 UV;
out vec3 Normal;

void main(){
	UV = _aUV;
	Normal = _aNormal;
	gl_Position = _uViewProjMat * vec4(_aPosition,1);
}