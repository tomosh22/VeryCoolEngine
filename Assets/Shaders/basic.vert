#version 450 core

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec4 _aColor;

out vec4 color;

void main(){
	color = _aColor;
	gl_Position = vec4(_aPosition,1);
}