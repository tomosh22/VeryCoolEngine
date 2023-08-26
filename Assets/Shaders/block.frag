#version 450 core


layout(location = 0) out vec4 _oColor;
in vec2 UV;
in vec3 Normal;
in vec3 WorldPos;
in vec3 Tangent;
in vec3 Binormal;
in flat ivec2 AtlasOffset;

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

layout(std140, binding=1) uniform LightsUBO{
uint numLights;
uint pad0;
uint pad1;
uint pad2;
Light lights[100];
};


uniform sampler2D diffuseTex;
uniform sampler2D bumpMap;
layout(rgba32f) uniform writeonly image2D debugTex;

//uniform ivec2 _uAtlasOffset;

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
	vec3 bumpNormal = normalize(normalize(TBN * texture2D(bumpMap,UV).rgb)*2-1);
	_oColor = vec4(0);
	vec4 diffuse = texture2D(diffuseTex,UV);
	for(int i = 0; i < numLights; i++){
		point(_oColor,diffuse,lights[i], bumpNormal);
	}
	_oColor.rgb += diffuse.rgb * 0.1f;

	vec2 atlasUV = UV/16;
	atlasUV += AtlasOffset * 1./16.;

	_oColor = texture2D(diffuseTex,atlasUV);
	if(AtlasOffset == ivec2(0,0)) _oColor *= vec4(0.5,1,0.5,1);
	//_oColor = vec4(UV,0,1);
}