#version 300 es

in lowp vec4 fragColor;

layout (location = 0) out lowp vec4 oColor;

void main(void)
{
	oColor = vec4(fragColor.rgb, 1.0);
}