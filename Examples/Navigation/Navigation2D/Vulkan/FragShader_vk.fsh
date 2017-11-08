#version 450
precision highp int; precision highp float;

layout(location = 0) out lowp vec4 oColour;

layout(std140, set = 1, binding = 0) uniform DynamicData
{
	uniform lowp vec4 myColor;
};

void main(void)
{
	oColour = vec4(myColor.rgb, 1.0);
}
