#version 450 core

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec2 _aUV;
layout(location = 2) in vec3 _aNormal;
layout(location = 3) in vec3 _aTangent;
layout(location = 4) in vec3 _aBitangent;


layout(std140, set = 0, binding=0) uniform matrices{
	mat4 _uViewMat;
	mat4 _uProjMat;
	mat4 _uViewProjMat;
	vec4 _uCamPos;//4 bytes of padding
};

layout(push_constant) uniform PushConstantVert{
	mat4 modelMatrix;
};

layout(location = 0) out vec2 UV;
layout(location = 1) out vec3 Normal;
layout(location = 2) out vec3 WorldPos;
layout(location = 3) out mat3 TBN;

void main(){
	mat4 mvp = _uViewProjMat * modelMatrix;
	WorldPos = (modelMatrix * vec4(_aPosition,1)).xyz;
	Normal = _aNormal;
	UV = _aUV * 5;
	
	mat3 normalMatrix = transpose ( inverse ( mat3 ( modelMatrix )));
	
	vec3 T = normalize(vec3(modelMatrix * vec4(_aTangent,0)));
    vec3 B = normalize(vec3(modelMatrix * vec4(_aBitangent,0)));
    vec3 N = normalize(vec3(modelMatrix * vec4(_aNormal,0)));
    TBN = mat3(T, B, N);
	
	


	gl_Position = mvp * vec4(_aPosition,1);
}