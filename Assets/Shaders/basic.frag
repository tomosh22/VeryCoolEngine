#version 450 core


layout(location = 0) out vec4 _oColor;
in vec4 color;

void main(){
	_oColor = color;
}