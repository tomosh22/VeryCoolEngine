#version 450 core

#include "Mesh_NoSkin_Common.h"

layout(location = 0) out vec4 _oColor;

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

layout(std140, set = 0, binding = 2) uniform Misc{
	vec3 overrideNormal;
	int useBumpMap;
	int usePhongTess;
	float phongTessFactor;
	int tessLevel;
};

layout(std140, set = 0, binding = 1) uniform LightsUBO{
uint numLights;
uint pad0;
uint pad1;
uint pad2;
Light lights[100];
};

layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2D bumpMap;
layout(set = 1, binding = 2) uniform sampler2D roughnessTex;
layout(set = 1, binding = 3) uniform sampler2D metallicTex;

void point(inout vec4 finalColor, vec4 diffuse, Light light, vec3 bumpNormal, float metal, float rough, float reflectivity) {
	vec3 lightDir = normalize(light.positionAndRadius.xyz - WorldPos);
	vec3 viewDir = normalize(_uCamPos.xyz - WorldPos);
	vec3 halfDir = normalize(lightDir + viewDir);

	float normalDotLightDir = max(dot(bumpNormal, lightDir), 0.0001);
	float normalDotViewDir = max(dot(bumpNormal, viewDir), 0.0001);
	float normalDotHalfDir = max(dot(bumpNormal, halfDir), 0.0001);
	float halfDirDotViewDir = max(dot(halfDir, viewDir), 0.0001);

	float F = reflectivity + (1 - reflectivity) * pow((1-halfDirDotViewDir), 5);

	F = 1 - F;
	


	float D = pow(rough, 2) / (3.14 * pow((pow(normalDotHalfDir, 2) * (pow(rough, 2) - 1) + 1), 2));

	

	float k = pow(rough + 1, 2) / 8;
	float G = normalDotViewDir / (normalDotViewDir * (1 - k) + k);

	float DFG = D * F * G;

	vec4 surface = diffuse * vec4(light.color.xyz, 1) * light.color.a;
	vec4 C = surface * (1 - metal);

	vec4 BRDF = ((1 - F) * (C / 3.14)) + (DFG / (4 * normalDotLightDir * normalDotViewDir));

	float dist = length(light.positionAndRadius.xyz - WorldPos);
	float atten = 1.0 - clamp(dist / light.positionAndRadius.w , 0.0, 1.0);

	finalColor += BRDF * normalDotLightDir * atten * light.color;
	//if(isnan(BRDF.x) || isnan(BRDF.y) || isnan(BRDF.z))finalColor = vec4(1,0,0,0);
	//else finalColor = vec4(0,1,0,0);
	finalColor.a = 1;
}

void main(){
	
	
	vec3 bumpNormal = texture(bumpMap, UV).rgb;
	bumpNormal += overrideNormal;
	bumpNormal = normalize(TBN * bumpNormal); 
	
	
	float roughness = texture(roughnessTex,UV).x;
	float metallic = texture(metallicTex,UV).x;
	_oColor = vec4(0);
	vec4 diffuse = texture(diffuseTex,UV);
	for(int i = 0; i < numLights; i++){
		
		if(useBumpMap != 0){
			point(_oColor, diffuse, lights[i], bumpNormal, metallic, roughness, 0.95);
		}
		else{
			point(_oColor, diffuse, lights[i], Normal, 0, roughness, 0.95);
		}
	}
	_oColor.rgb += diffuse.rgb * 0.5f;
	_oColor.a = 1.f;
	
	if(bSelected > 0)
		_oColor.rgb *= vec3(1,0,0);
}