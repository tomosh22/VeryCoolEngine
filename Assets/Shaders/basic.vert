#version 450 core

layout(location = 0) in vec3 _aPosition;

void main(){
	gl_Position = vec4(_aPosition,1);
}