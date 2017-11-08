#version 310 es

precision highp float;

layout(location=0) out vec4 fullDimensionOutColor;
layout(location=1) out vec4 halfDimensionOutColor;

void main()
{
    halfDimensionOutColor = fullDimensionOutColor = vec4(.35, 0.0, 0.50, 1.0);
}