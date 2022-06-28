#version 320 es

layout(location = 0) in highp vec2 vTexcoord;
layout(location = 0) out highp vec4 outColor;

layout(set = 0, binding = 0) uniform highp sampler2D computeOutput;

void main() { outColor = texture(computeOutput, vTexcoord); }