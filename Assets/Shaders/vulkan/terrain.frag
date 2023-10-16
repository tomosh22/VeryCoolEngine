#version 450 core


layout(location = 0) out vec4 _oColor;

layout(location = 0) in vec2 UV;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 WorldPos;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Binormal;

struct Light{
	vec4 positionAndRadius;
	vec4 color;
};

layout(std140, binding=0) uniform matrices{
	mat4 _uViewMat;
	mat4 _uProjMat;
	mat4 _uViewProjMat;
	vec4 _uCamPos;//4 bytes of padding
};

layout(std140, set = 2, binding = 0) uniform LightsUBO{
uint numLights;
uint pad0;
uint pad1;
uint pad2;
Light lights[100];
};

layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2D bumpMap;

void point(inout vec4 finalColor, vec4 diffuse, Light light, vec3 bumpNormal) {
	
	vec3 lightDir = normalize(light.positionAndRadius.xyz - WorldPos);
	vec3 viewDir = normalize(_uCamPos.xyz - WorldPos);
	vec3 halfDir = normalize(lightDir + viewDir);

	float lambert = max(dot(lightDir,bumpNormal),0.f);
	float distance = length(light.positionAndRadius.xyz - WorldPos);
	float atten = 1.f - clamp(distance/light.positionAndRadius.w,0.f,1.f);

	float specular = pow(clamp(dot(halfDir,bumpNormal),0.f,1.f),120.f);
	

	vec3 surface = diffuse.rgb * light.color.rgb;
	finalColor.rgb += surface * lambert * atten;
	finalColor.rgb += (light.color.rgb * specular) * atten/0.3f;
	
	finalColor.a += diffuse.a;
}

void main(){
	mat3 TBN = mat3 (normalize(Tangent),normalize(Binormal),normalize(Normal));
	vec3 bumpNormal = normalize(normalize(TBN * texture(bumpMap,UV).rgb)*2-1);
	_oColor = vec4(0);
	vec4 diffuse = texture(diffuseTex,UV);
	for(int i = 0; i < numLights; i++){
		point(_oColor,diffuse,lights[i], bumpNormal);
	}
	_oColor.rgb += diffuse.rgb * 0.1f;
	//_oColor.rgb = Normal;
	_oColor.a = 1.f;
	bumpNormal.x *= 10.f;
	bumpNormal.y = 0;
	bumpNormal.z *= 10.f;
	//_oColor = vec4(bumpNormal,1);
	
}