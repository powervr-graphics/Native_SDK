#version 450

layout(location = 0) out lowp vec4 oColour;

layout(location = 0) in lowp vec4 fragColour;

void main(void)
{
	oColour = vec4(fragColour.rgb, 1.0);
}