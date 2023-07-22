#version 450 core

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec2 _aUV;

uniform mat4 viewMat;
uniform mat4 projMat;

layout(std140, binding=5) uniform matrices{ mat4 _uViewProjMat;};

out vec4 color;

void main(){
	color = vec4(_aUV,0,1);
	gl_Position = _uViewProjMat * vec4(_aPosition,1);
}