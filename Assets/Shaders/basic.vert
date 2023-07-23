#version 450 core

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec2 _aUV;


layout(std140, binding=5) uniform matrices{ mat4 _uViewProjMat;};

out vec2 UV;

void main(){
	UV = _aUV;
	gl_Position = _uViewProjMat * vec4(_aPosition,1);
}