#version 450 core

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec2 _aUV;
layout(location = 2) in vec4 _aInstanceQuat;
layout(location = 3) in vec3 _aInstancePosition;
layout(location = 4) in ivec2 _aInstanceAtlasOffset;
layout(location = 5) in ivec4 _ainstanceAOValues;//one value per face vertex, hence 4 components

uniform mat4 _uModelMat;

layout(std140, binding=0) uniform matrices{
	mat4 _uViewMat;
	mat4 _uProjMat;
	mat4 _uViewProjMat;
	vec4 _uCamPos;//vec3 plus 4 bytes of padding
};

out vec2 UV;
out vec3 Normal;
out vec3 WorldPos;
out vec3 Tangent;
out vec3 Binormal;
out flat ivec2 AtlasOffset;
out float AO;

vec3 RotateVertex(vec3 vertex, vec4 quat) {
	return vertex + 2.0 * cross(quat.xyz, cross(quat.xyz, vertex) + quat.w * vertex);
    vec3 qvec = quat.xyz;
    float qscalar = quat.w;
    
    vec3 uv = cross(qvec, vertex) + qscalar * vertex;
    uv += cross(qvec, uv) * 2.0;
    
    return uv;
}

void main(){
	//WorldPos = (_aInstanceMat * vec4(_aPosition,1)).xyz;
	WorldPos = _aPosition;
	WorldPos = RotateVertex(WorldPos, _aInstanceQuat.wzyx);
	WorldPos = _aInstancePosition + WorldPos;
	//WorldPos = RotateVertex(WorldPos, _aInstanceQuat);
	UV = _aUV;
	//mat3 normalMat = mat3(1);//#todo transpose(inverse(mat3(modelMatrix)));
	//Normal = normalize(normalMat * normalize(_aNormal));
	//vec3 wNormal = normalize(normalMat * normalize(_aNormal));
	//vec3 wTangent = normalize(normalMat * normalize(_aTangent.xyz));
	//Tangent = wTangent;
	//Binormal = cross(wTangent,wNormal) * _aTangent.w;

	AtlasOffset = _aInstanceAtlasOffset;

	if(gl_VertexID == 0) AO = _ainstanceAOValues.x;
	if(gl_VertexID == 1) AO = _ainstanceAOValues.y;
	if(gl_VertexID == 2) AO = _ainstanceAOValues.z;
	if(gl_VertexID == 3) AO = _ainstanceAOValues.w;



	gl_Position = _uViewProjMat * vec4(WorldPos,1);//#todo model mat
}