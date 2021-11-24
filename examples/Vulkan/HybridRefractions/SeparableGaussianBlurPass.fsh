#version 320 es

layout(set = 0, binding = 0) uniform mediump sampler2D rayTracedRefractions;

layout(location = 0) in mediump vec2 inUV[2];
layout(location = 2) in mediump float inWeight[2];

layout(location = 0) out mediump vec4 outFragColor;

void main()
{
	outFragColor = texture(rayTracedRefractions, inUV[0]) * inWeight[0] + texture(rayTracedRefractions, inUV[1]) * inWeight[1];
}
