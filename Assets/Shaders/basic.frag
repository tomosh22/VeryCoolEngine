#version 450 core


layout(location = 0) out vec4 _oColor;
in vec2 UV;
in vec3 Normal;

struct Light{
	vec4 positionAndRadius;
	vec4 color;
};

layout(std140, binding=1) uniform LightsUBO{
uint numLights;
uint pad0;
uint pad1;
uint pad2;
Light lights[100];
};

uniform sampler2D texture;

void main(){
	for(int i = 0; i < numLights; i++){
		_oColor += lights[i].color;
	}
	_oColor /= numLights;
	//_oColor = texture2D(texture,UV);
}