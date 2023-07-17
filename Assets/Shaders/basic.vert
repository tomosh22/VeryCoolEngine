#version 450 core

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec4 _aColor;

uniform mat4 viewMat;
uniform mat4 projMat;

out vec4 color;

void main(){
	color = _aColor;
	gl_Position = projMat * viewMat * vec4(_aPosition,1);
}