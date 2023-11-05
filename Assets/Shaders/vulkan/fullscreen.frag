#version 450 core

layout(std140, binding=0) uniform matrices{
	mat4 _uViewMat;
	mat4 _uProjMat;
	mat4 _uViewProjMat;
	vec4 _uCamPos;//4 bytes of padding
};


//layout(set = 0, binding = 0) uniform samplerCube cubemap;


layout(location = 0) out vec4 _oColor;
layout(location = 1) out vec4 _oSceneEditorTex;

layout(location = 0) in vec2 UV;

vec3 RayDir(vec2 pixel)//takes pixel in 0,1 range
{
	vec2 ndc = pixel * 2 - 1;//move from 0,1 to -1,1

	vec4 clipSpace = vec4(ndc, -1,1);

	vec4 viewSpace = inverse(_uProjMat) * clipSpace; //#TODO invert this CPU side
	viewSpace.w = 0;

	vec3 worldSpace = (inverse(_uViewMat) * viewSpace).xyz; //#TODO same here

	return normalize(worldSpace);
}

void main(){
	vec3 rayDir = RayDir(UV);

	//_oColor = texture(cubemap, rayDir);

	_oColor = vec4(0.2,0.3,0.9,1);
	_oColor += max(rayDir.y + 0.3,0);
	//_oColor = vec4(rayDir+0.5,1);
	
	_oSceneEditorTex = _oColor;
}