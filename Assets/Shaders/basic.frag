#version 450 core

uniform int numLights = 1;//#todo update

layout(location = 0) out vec4 _oColor;
in vec2 UV;
in vec3 Normal;

struct Light{
	vec3 position;
	float radius;
	vec3 color;
};

layout(std140, binding=1) uniform LightsUBO{
Light lights[];};

uniform sampler2D texture;

void main(){
	_oColor = vec4(lights[0].color,1);
	//_oColor = texture2D(texture,UV);
}