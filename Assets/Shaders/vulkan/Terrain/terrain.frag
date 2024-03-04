#version 450 core

#include "Terrain_Common.h"

layout(location = 0) out vec4 _oColor;

layout(location = 0) in vec2 UV;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 WorldPos;
#ifdef USE_TESSELATION
layout(location = 3) in mat3 TBN;
#else
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Bitangent;
#endif



void point(inout vec4 finalColor, vec4 diffuse, PointLight light, vec3 bumpNormal, float metal, float rough, float reflectivity) {
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
	finalColor.a = 1;
}

void directional(inout vec4 finalColor, vec4 diffuse, DirectionalLight light, vec3 bumpNormal, float metal, float rough, float reflectivity) {
    vec3 lightDir = normalize(-light.direction); // Directional light direction is already normalized.
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

    // No distance attenuation for directional light

    finalColor += BRDF * normalDotLightDir * light.color;
    finalColor.a = 1;
}

void main(){



	vec2 uv = UV * uvScale;
	
	
	vec3 bumpNormal = texture(bumpMap, uv).rgb;
	bumpNormal = normalize(TBN * bumpNormal); 
	#ifndef USE_TESSELATION
	if(useBumpMap == 0){
		bumpNormal = normal;
	}
	#endif
	
	if(visualiseNormals != 0){
		_oColor = vec4(bumpNormal,1);
		return;
	}
	
	float roughness = texture(roughnessTex,uv).x;
	float metallic = texture(metallicTex,uv).x;
	_oColor = vec4(0);
	vec4 diffuse = texture(diffuseTex,uv);
	for(int i = 0; i < numLights; i++){
		point(_oColor, diffuse, lights[i], bumpNormal, metallic, roughness, fReflectivity);
		
	}
	DirectionalLight xDirLight;
	xDirLight.direction = vec3(0.2,-0.8,0.2);
	xDirLight.color = vec4(1);
	directional(_oColor, diffuse, xDirLight, bumpNormal, metallic, roughness, fReflectivity);
	//_oColor.rgb += diffuse.rgb * 0.5f;
	_oColor.a = 1.f;
	
	if(bSelected > 0)
		_oColor.rgb *= vec3(1,0,0);
}