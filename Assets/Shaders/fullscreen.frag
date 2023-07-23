#version 450 core

uniform mat4 inverseProjMatrix;
uniform mat4 inverseViewMatrix;

uniform vec3 cameraPos;

uniform samplerCube cubemap;

layout(rgba32f) uniform writeonly image2D debugTex;

layout(location = 0) out vec4 _oColor;
in vec2 UV;

vec3 RayDir(vec2 pixel)//takes pixel in 0,1 range
{
	vec2 ndc = pixel * 2 - 1;//move from 0,1 to -1,1

	vec4 clipSpace = vec4(ndc, -1,1);

	vec4 viewSpace = inverseProjMatrix * clipSpace;
	viewSpace.w = 0;

	vec3 worldSpace = (inverseViewMatrix * viewSpace).xyz;

	return normalize(worldSpace);
}

void main(){
	vec3 rayDir = RayDir(UV);

	_oColor = texture(cubemap, (rayDir));
}