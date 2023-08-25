#version 450 core

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec2 _aUV;
layout(location = 2) in mat4 _aInstanceMat;
layout(location = 6) in vec3 _aInstancePosition;
layout(location = 7) in ivec2 _aInstanceAtlasOffset;

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

vec3 RotateVertex(vec3 vertex, vec4 quat) {
    //vec3 qvec = quaternion.xyz;
    //float qscalar = quaternion.w;
    //
    //vec3 uv = cross(qvec, vertex) + qscalar * vertex;
    //uv += cross(qvec, uv) * 2.0;
    //
    //return uv;

	mat4 mat;
	float yy = quat.y * quat.y;
	float zz = quat.z * quat.z;
	float xy = quat.x * quat.y;
	float zw = quat.z * quat.w;
	float xz = quat.x * quat.z;
	float yw = quat.y * quat.w;
	float xx = quat.x * quat.x;
	float yz = quat.y * quat.z;
	float xw = quat.x * quat.w;

	mat[0][0] = 1 - 2 * yy - 2 * zz;
	mat[0][1] = 2 * xy + 2 * zw;
	mat[0][2] = 2 * xz - 2 * yw;

	mat[1][0] = 2 * xy - 2 * zw;
	mat[1][1] = 1 - 2 * xx - 2 * zz;
	mat[1][2] = 2 * yz + 2 * xw;

	mat[2][0] = 2 * xz + 2 * yw;
	mat[2][1] = 2 * yz - 2 * xw;
	mat[2][2] = 1 - 2 * xx - 2 * yy;

	mat[0][3] = 0;
	mat[1][3] = 0;
	mat[2][3] = 0;
	mat[3][3] = 1;

	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;

	return (mat * vec4(vertex,1)).xyz;
}

void main(){
	//WorldPos = (_aInstanceMat * vec4(_aPosition,1)).xyz;
	WorldPos = _aPosition;
	WorldPos = (_aInstanceMat * vec4(WorldPos,1)).xyz;
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



	gl_Position = _uViewProjMat * vec4(WorldPos,1);//#todo model mat
}