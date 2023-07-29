#version 450 core


layout(location = 0) out vec4 _oColor;
in vec2 UV;
in vec3 Normal;

uniform sampler2D texture;

void main(){
	_oColor = vec4(Normal,1);
	//_oColor = texture2D(texture,UV);
}