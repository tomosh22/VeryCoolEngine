#version 450 core


layout(location = 0) out vec4 _oColor;
in vec2 UV;

uniform sampler2D texture;

void main(){
	_oColor = texture2D(texture,UV);
}