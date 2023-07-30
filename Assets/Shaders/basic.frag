#version 450 core


layout(location = 0) out vec4 _oColor;
in vec2 UV;
in vec3 Normal;
in vec3 WorldPos;

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

uniform sampler2D texture;
layout(rgba32f) uniform writeonly image2D debugTex;

void point(inout vec4 finalColor, vec4 diffuse, Light light) {
	vec3 lightDir = normalize(light.positionAndRadius.xyz - WorldPos);
	vec3 viewDir = normalize(_uCamPos.xyz - WorldPos);
	vec3 halfDir = normalize(lightDir + viewDir);

	float lambert = max(dot(lightDir,Normal),0.f);
	float distance = length(light.positionAndRadius.xyz - WorldPos);
	float atten = 1.f - clamp(distance/light.positionAndRadius.w,0.f,1.f);

	float specular = pow(clamp(dot(halfDir,Normal),0.f,1.f),60.f);

	vec3 surface = diffuse.rgb * light.color.rgb;
	finalColor.rgb += surface * lambert * atten;
	finalColor.rgb += (light.color.rgb * specular) * atten/0.3f;
	finalColor.rgb += surface * 0.1f;
	finalColor.a += diffuse.a;
}

void main(){
	_oColor = vec4(0);
	vec4 diffuse = texture2D(texture,UV);
	for(int i = 0; i < numLights; i++){
		point(_oColor,diffuse,lights[i]);
	}

	//Light light = lights[0];
	//bool isWithinLight = length(WorldPos - light.positionAndRadius.xyz) < light.positionAndRadius.w;
	//_oColor = isWithinLight ? vec4(1) : vec4(0);
	//_oColor = vec4(length(WorldPos - light.positionAndRadius.xyz)/100.f);
	//_oColor = texture2D(texture,UV);
	//ivec2 coord = ivec2(1280 * UV.x,720 * UV.y);
	//imageStore(debugTex,coord,vec4(length(WorldPos - light.positionAndRadius.xyz),light.positionAndRadius.w,0,1 ));
}