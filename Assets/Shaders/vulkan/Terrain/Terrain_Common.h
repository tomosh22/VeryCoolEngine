layout(push_constant) uniform ModelMatrix{
	mat4 modelMatrix;
	int bSelected;
};

layout(std140, set = 0, binding=0) uniform matrices{
	mat4 _uViewMat;
	mat4 _uProjMat;
	mat4 _uViewProjMat;
	vec4 _uCamPos;//4 bytes of padding
};

struct PointLight{
	vec4 positionAndRadius;
	vec4 color;
};
struct DirectionalLight{
    vec3 direction;
    vec4 color;
};
layout(std140, set = 0, binding = 1) uniform LightsUBO{
	uint numLights;
	uint pad0;
	uint pad1;
	uint pad2;
	PointLight lights[100];
};

layout(std140, set = 0, binding = 2) uniform Misc{
	int useBumpMap;
	int visualiseNormals;
	int tessLevel;
	float heightMultiplier;
	int uvScale;
};



layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2D bumpMap;
layout(set = 1, binding = 2) uniform sampler2D roughnessTex;
layout(set = 1, binding = 3) uniform sampler2D metallicTex;
layout(set = 1, binding = 4) uniform sampler2D detailHeightmap;

const float fReflectivity = 0.99f;

#define USE_TESSELATION