#version 450 core

layout(set = 1, binding = 5) uniform sampler2D heightmapTex;

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec2 _aUV;
layout(location = 2) in vec3 _aInstancePosition;


layout(std140, set = 0, binding=0) uniform matrices{
	mat4 _uViewMat;
	mat4 _uProjMat;
	mat4 _uViewProjMat;
	vec4 _uCamPos;//vec3 plus 4 bytes of padding
};

layout(location = 0) out vec2 UV;
layout(location = 1)out vec3 WorldPos;



void main(){

	bool bBillboard = true;
	
	WorldPos = _aInstancePosition + _aPosition;
	UV = _aUV;
	
	if(bBillboard){
		vec3 viewDir = normalize(_uCamPos.xyz - WorldPos);
		vec3 upDir = vec3(0.0, 1.0, 0.0);
		
		mat3 billboardMat = mat3(
			cross(upDir, viewDir),
			upDir,
			-viewDir
		);
		
		vec3 rotatedPosition = billboardMat * _aPosition;
	
		gl_Position = _uProjMat * _uViewMat * vec4(_aInstancePosition + rotatedPosition, 1.0);
	}
	else{
		gl_Position = _uViewProjMat * vec4(_aInstancePosition + _aPosition, 1);
	}
}
