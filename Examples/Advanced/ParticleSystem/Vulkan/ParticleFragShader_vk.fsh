#version 450

layout (location = 0) in mediump float lifespan;
layout (location = 0) out lowp vec4 oColor;

void main()
{
	oColor = vec4(.5, lifespan, 1., 1.);
}
