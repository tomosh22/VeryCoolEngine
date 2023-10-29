#version 450 core

layout(triangles , ccw) in;

layout(location = 0) in vec2 _aUV[];
layout(location = 1) in vec3 _aNormal[];
layout(location = 2) in vec3 _aTangent[];
layout(location = 3) in vec3 _aBitangent[];
layout(location = 4) in vec3 _aWorldPos[];

layout(location = 0) out vec2 _oUV;
layout(location = 1) out vec3 _oNormal;
layout(location = 2) out vec3 _oWorldPos;
layout(location = 3) out mat3 _oTBN;


layout(std140, set = 0, binding=0) uniform matrices{
	mat4 _uViewMat;
	mat4 _uProjMat;
	mat4 _uViewProjMat;
	vec4 _uCamPos;//4 bytes of padding
};

layout(push_constant) uniform PushConstantVert{
	mat4 modelMatrix;
	vec3 overrideNormal;
	int useBumpMap;
};

vec3 TriMixVec3(vec3 a, vec3 b, vec3 c) {
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	vec3 val0 = a;
	vec3 val1 = b;
	vec3 val2 = c;

	vec3 val = u * val0 + v * val1 + w * val2;
	return val;
}

vec2 TriMixVec2(vec2 a, vec2 b, vec2 c) {
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	vec2 val0 = a;
	vec2 val1 = b;
	vec2 val2 = c;

	vec2 val = u * val0 + v * val1 + w * val2;
	return val;
}

layout(set = 2, binding = 4) uniform sampler2D heightMap;

void main(){
	vec3 combinedPos = TriMixVec3(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz, gl_in[2].gl_Position.xyz);
	
	_oUV = TriMixVec2(_aUV[0], _aUV[1], _aUV[2]);
	_oNormal = TriMixVec3(_aNormal[0], _aNormal[1], _aNormal[2]);
	_oWorldPos = TriMixVec3(_aWorldPos[0], _aWorldPos[1], _aWorldPos[2]);
	
	vec3 combinedTangent = TriMixVec3(_aTangent[0], _aTangent[1], _aTangent[2]);
	vec3 combinedBitangent = TriMixVec3(_aBitangent[0], _aBitangent[1], _aBitangent[2]);
	
	vec4 worldPos = modelMatrix * vec4(combinedPos , 1);
	
	float height = texture(heightMap , _oUV ).x;
	worldPos.xyz += _oNormal * height;
	
	
	gl_Position = _uViewProjMat * worldPos;
	
	
	
	
	
	mat3 normalMatrix = transpose ( inverse ( mat3 ( modelMatrix )));
	
	vec3 T = normalize(vec3(modelMatrix * vec4(combinedTangent,0)));
    vec3 B = normalize(vec3(modelMatrix * vec4(combinedBitangent,0)));
    vec3 N = normalize(vec3(modelMatrix * vec4(_oNormal,0)));
    _oTBN = mat3(T, B, N);
	
}