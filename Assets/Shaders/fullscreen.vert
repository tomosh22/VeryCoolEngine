#version 450 core

layout(location = 0) in vec3 _aPosition;
layout(location = 1) in vec2 _aUV;


out vec2 UV;

void main(){
    uint xBit = gl_VertexID & 1u;
    uint yBit = (gl_VertexID >> 1) & 1u;

    float xPos = float(xBit) * 2.0 - 1.0;
    float yPos = float(yBit) * 2.0 - 1.0;

    gl_Position = vec4(xPos, yPos, 0.0, 1.0);

    UV = vec2((xPos + 1.0) * 0.5, (yPos + 1.0) * 0.5);
}