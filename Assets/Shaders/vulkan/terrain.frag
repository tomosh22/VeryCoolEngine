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

layout(std140, set = 0, binding=0) uniform matrices{
	mat4 _uViewMat;
	mat4 _uProjMat;
	mat4 _uViewProjMat;
	vec4 _uCamPos;//4 bytes of padding
};

layout(push_constant) uniform PushConstantVert{
	mat4 modelMatrix;
	int useBumpMap;
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

void phong(inout vec4 finalColor, vec4 diffuse, Light light, vec3 normal){
	vec3 lightDir = normalize(light.positionAndRadius.xyz - WorldPos);
	vec3 viewDir = normalize(_uCamPos.xyz - WorldPos);
	vec3 halfDir = normalize(lightDir + viewDir);
	float lambert = max(dot(lightDir, normal),0);
	float distance = length (light.positionAndRadius.xyz - WorldPos);
    float atten = 1.0f - clamp( distance / light.positionAndRadius.w ,0.0 ,1.0);
	float specFactor = clamp(dot(halfDir,normal),0,1);
	specFactor = pow(specFactor,30);
	vec3 surface = (diffuse.rgb * light.color.rgb);
	finalColor.rgb += surface * lambert * atten;
	finalColor.rgb += (light.color.rgb * specFactor) * atten * 0.33;
	finalColor.rgb += surface * 0.1;
	finalColor.a = diffuse.a;
}

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


vec3 CookTorrance(vec3 materialDiffuseColor,
	vec3 materialSpecularColor,
	vec3 normal,
	vec3 lightDir,
	vec3 viewDir,
	vec3 lightColor,
	float roughness,
	float atten)
{
	float F0 = 0.8;
	float PI = 3.14159265f;
	float k = 0.2;

	float NdotL = max(0, dot(normal, lightDir));
	float Rs = 0.0;
	if (NdotL > 0) 
	{
		vec3 H = normalize(lightDir + viewDir);
		float NdotH = max(0, dot(normal, H));
		float NdotV = max(0, dot(normal, viewDir));
		float VdotH = max(0, dot(lightDir, H));

		// Fresnel reflectance
		float F = pow(1.0 - VdotH, 5.0);
		F *= (1.0 - F0);
		F += F0;

		// Microfacet distribution by Beckmann
		float m_squared = roughness * roughness;
		float r1 = 1.0 / (4.0 * m_squared * pow(NdotH, 4.0));
		float r2 = (NdotH * NdotH - 1.0) / (m_squared * NdotH * NdotH);
		float D = r1 * exp(r2);

		// Geometric shadowing
		float two_NdotH = 2.0 * NdotH;
		float g1 = (two_NdotH * NdotV) / VdotH;
		float g2 = (two_NdotH * NdotL) / VdotH;
		float G = min(1.0, min(g1, g2));

		Rs = (F * D * G) / (PI * NdotL * NdotV);
	}
	return materialDiffuseColor * lightColor * NdotL + lightColor * materialSpecularColor * NdotL * (k + Rs * (1.0 - k)) * atten;
}

void main(){
	mat3 TBN = mat3 (normalize(Tangent),normalize(Binormal),normalize(Normal));
	vec3 bumpNormal = vec3(0,0,1);//texture(bumpMap, UV ).rgb;
	bumpNormal = normalize( TBN * normalize( bumpNormal * 2.0 - 1.0));
	float roughness = texture(roughnessTex,UV).x;
	_oColor = vec4(0);
	vec4 diffuse = texture(diffuseTex,UV);
	for(int i = 0; i < numLights; i++){
		//vec3 viewDir = normalize(_uCamPos.xyz - WorldPos);
		//vec3 lightDir = normalize(lights[i].positionAndRadius.xyz - WorldPos);
		//float dist = length(lights[i].positionAndRadius.xyz - WorldPos);
		//float atten = 1.0 - clamp(dist / lights[i].positionAndRadius.w , 0.0, 1.0);
		//_oColor.rgb += CookTorrance(diffuse.rgb, lights[i].color.rgb, bumpNormal, lightDir, viewDir, lights[i].color.rgb, roughness, atten);
		
		//point(_oColor,diffuse,lights[i], bumpNormal, 0, roughness, 0);
		if(useBumpMap != 0){
			phong(_oColor, diffuse, lights[i], bumpNormal);
		}
		else{
			phong(_oColor, diffuse, lights[i], Normal);
		}
	}
	_oColor.rgb += diffuse.rgb * 0.1f;
	_oColor.a = 1.f;
	if(useBumpMap != 0){
			_oColor.rgb = bumpNormal;
		}
		else{
			_oColor.rgb = Normal;
		}
		
	_oColor.rgb = Tangent;
	
}