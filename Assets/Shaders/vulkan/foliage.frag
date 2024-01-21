#version 450 core


layout(location = 0) out vec4 _oColor;

layout(location = 0) in vec2 UV;
layout(location = 1) in vec3 WorldPos;


layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2D bumpMap;
layout(set = 1, binding = 2) uniform sampler2D roughnessTex;
layout(set = 1, binding = 3) uniform sampler2D alphaTex;
layout(set = 1, binding = 4) uniform sampler2D translucencyTex;


void main(){
	float alpha = texture(alphaTex, UV).r;
	if(alpha < 0.9) discard;
	_oColor = vec4(texture(diffuseTex, UV).rgb, alpha);
	
	
}