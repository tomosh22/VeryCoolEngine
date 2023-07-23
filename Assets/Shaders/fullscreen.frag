#version 450 core

uniform mat4 projMatrix;
uniform mat4 viewMatrix;

layout(location = 0) out vec4 _oColor;
in vec2 UV;

vec3 RayDir(vec2 pixel)//takes pixel in 0,1 range
{
	vec2 flipped = vec2(1-pixel.y,  pixel.x);
	vec2 ndc = flipped * 2 - 1;//move from 0,1 to -1,1

	vec4 clipSpace = vec4(ndc, -1,1);


	vec4 viewSpace = inverse(projMatrix) * clipSpace;
	viewSpace.w = 0;
	//return viewSpace.xyz;

	vec3 worldSpace = (inverse(viewMatrix) * viewSpace).xyz;

	return normalize(worldSpace);
}

void main(){
	vec3 rayDir = RayDir(UV);


	_oColor = vec4(rayDir,1);
}