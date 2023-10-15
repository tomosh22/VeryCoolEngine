#version 450 core


layout(location = 0) out vec4 _oColor;

layout(location = 0) in vec2 UV;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 WorldPos;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Binormal;
layout(location = 5) in flat ivec2 AtlasOffset;
layout(location = 6) in float AO;

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


layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
//uniform sampler2D bumpMap;
//layout(rgba32f) uniform writeonly image2D debugTex;

//uniform bool aoEnabled;

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
	//mat3 TBN = mat3 (normalize(Tangent),normalize(Binormal),normalize(Normal));
	//vec3 bumpNormal = normalize(normalize(TBN * texture2D(bumpMap,UV).rgb)*2-1);
	//_oColor = vec4(0);
	//vec4 diffuse = texture2D(diffuseTex,UV);
	//for(int i = 0; i < numLights; i++){
	//	point(_oColor,diffuse,lights[i], bumpNormal);
	//}
	//_oColor.rgb += diffuse.rgb * 0.1f;
	//
	//vec2 atlasUV = UV/16;
	//atlasUV += AtlasOffset * 1./16.;
	//
	//_oColor = texture2D(diffuseTex,atlasUV);
	//if(AtlasOffset == ivec2(0,0)) _oColor *= vec4(0.569,0.741,0.5,349);
	//
	//if(aoEnabled)_oColor *= 1 - (AO/4.f);
	////_oColor = vec4(UV,0,1);
	
	_oColor = vec4(AO,0,0,1);
	_oColor = vec4(UV,0,1);
	vec2 atlasUV = UV/16;
	atlasUV += AtlasOffset * 1./16.;

	_oColor = texture(diffuseTex,atlasUV);
	if(AtlasOffset == ivec2(0,0)) _oColor *= vec4(0.569,0.741,0.5,1);
	_oColor *= 1 - (AO/4.f);
	_oColor.a = 1;
	//_oColor = vec4(UV,0,1);
}