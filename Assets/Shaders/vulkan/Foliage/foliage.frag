#version 450 core

#include "Foliage_Common.h"

layout(location = 0) out vec4 _oColor;

layout(location = 0) in vec2 UV;


void main(){
	float alpha = texture(alphaTex, UV).r;
	if(alpha < 0.9) discard;
	_oColor = vec4(texture(diffuseTex, UV).rgb, alpha);
	
	
}