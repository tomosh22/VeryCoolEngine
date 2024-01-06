#version 450 core

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec2 _aUV;
layout(location = 2) in vec3 _aNormal;
layout(location = 3) in vec3 _aTangent;
layout(location = 4) in vec3 _aBitangent;
layout(location = 5) in uvec4 _aBoneIndices0to4;
layout(location = 6) in uvec4 _aBoneIndices5to8;
layout(location = 7) in vec4 _aBoneWeights0to4;
layout(location = 8) in vec4 _aBoneWeights5to8;

layout(std140, set = 0, binding=0) uniform matrices{
	mat4 _uViewMat;
	mat4 _uProjMat;
	mat4 _uViewProjMat;
	vec4 _uCamPos;//4 bytes of padding
};


layout(std140, set = 1, binding=5) uniform Bones{
	mat4 _uBones[128];
};

layout(push_constant) uniform PushConstantSkinnedVert{
	mat4 modelMatrix;
	int animate;
	float alpha;
};

layout(location = 0) out vec2 _oUV;
layout(location = 1) out vec3 _oNormal;
layout(location = 2) out vec3 _oWorldPos;
layout(location = 3) out mat3 TBN;

void main(){
	mat4 bone = _uBones[0];
	_oUV = _aUV * 5;
	_oNormal = _aNormal;
	
	
	vec4 localPos = vec4(_aPosition,1);
	vec4 skelPos = vec4(0,0,0,0);
	if(animate > 0){
		for(int i = 0; i < 4; i++){
			uint boneIndex = _aBoneIndices0to4[i];
			float boneWeight = _aBoneWeights0to4[i];
			skelPos += inverse(_uBones[boneIndex]) * localPos * boneWeight;
		}
		for(int i = 0; i < 4; i++){
			uint boneIndex = _aBoneIndices5to8[i];
			float boneWeight = _aBoneWeights5to8[i];
			skelPos += inverse(_uBones[boneIndex]) * localPos * boneWeight;
		}
	}
	else{
		skelPos = vec4(_aPosition,1); 
	}
	
	skelPos = mix(vec4(_aPosition,1), skelPos,alpha);
	
	_oWorldPos = (modelMatrix * vec4(skelPos.xyz,1)).xyz;
	vec3 T = normalize(vec3(modelMatrix * vec4(_aTangent,0)));
    vec3 B = normalize(vec3(modelMatrix * vec4(_aBitangent,0)));
    vec3 N = normalize(vec3(modelMatrix * vec4(_oNormal,0)));
    TBN = mat3(T, B, N);

	gl_Position = _uViewProjMat * vec4(_oWorldPos,1);
}