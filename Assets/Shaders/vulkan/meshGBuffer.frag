#version 450 core

layout(location = 0) out vec4 _oDiffuse;
layout(location = 1) out vec4 _oNormal;
layout(location = 2) out vec4 _oMaterial;

layout(location = 0) in vec2 UV;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 WorldPos;
layout(location = 3) in mat3 TBN;

struct Light{
	vec4 positionAndRadius;
	vec4 color;
};

layout(std140, set = 0, binding=0) uniform matrices{
	mat4 _uViewMat;
	mat4 _uProjMat;
	mat4 _uViewProjMat;
	vec4 _uCamPos;//4 bytes of padding
};


layout(std140, set = 1, binding = 0) uniform LightsUBO{
uint numLights;
uint pad0;
uint pad1;
uint pad2;
Light lights[100];
};

layout(set = 2, binding = 0) uniform sampler2D diffuseTex;
layout(set = 2, binding = 1) uniform sampler2D bumpMap;
layout(set = 2, binding = 2) uniform sampler2D roughnessTex;
layout(set = 2, binding = 3) uniform sampler2D metallicTex;




void main(){
	
	_oDiffuse = texture(diffuseTex,UV);
	
	
	vec3 bumpNormal = texture(bumpMap, UV).rgb;
	bumpNormal = normalize(TBN * bumpNormal); 
	_oNormal = vec4(bumpNormal, 1);
	
	float roughness = texture(roughnessTex,UV).x;
	float metallic = texture(metallicTex,UV).x;
	
	_oMaterial = vec4(roughness, metallic,0,1);
	
}