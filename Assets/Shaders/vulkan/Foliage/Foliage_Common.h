layout(std140, set = 0, binding=0) uniform matrices{
	mat4 _uViewMat;
	mat4 _uProjMat;
	mat4 _uViewProjMat;
	vec4 _uCamPos;//vec3 plus 4 bytes of padding
};


layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2D bumpMap;
layout(set = 1, binding = 2) uniform sampler2D roughnessTex;
layout(set = 1, binding = 3) uniform sampler2D alphaTex;
layout(set = 1, binding = 4) uniform sampler2D translucencyTex;
layout(set = 1, binding = 5) uniform sampler2D heightmapTex;