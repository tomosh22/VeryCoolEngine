#version 450 core

layout(location = 0) out vec4 _oColor;

layout(location = 0) in vec2 UV;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 WorldPos;
layout(location = 3) in mat3 TBN;

#include "mesh.frag"


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
			point(_oColor, diffuse, lights[i], bumpNormal, metallic, roughness, 0.5);
		}
		else{
			point(_oColor, diffuse, lights[i], Normal, 0, roughness, 0.5);
		}
	}
	_oColor.rgb += diffuse.rgb * 0.5f;
	_oColor.a = 1.f;
	
	
}