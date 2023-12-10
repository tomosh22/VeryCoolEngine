#version 450 core



layout(location = 0) out vec4 _oColor;

layout(location = 0) in vec2 UV;

layout(set = 1, binding = 0) uniform sampler2D diffuseTex;

void main(){

	_oColor = texture(diffuseTex, UV);
	
}