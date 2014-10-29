#version 300 es

in highp float varTimeToLive;

layout(location = 0) out lowp vec4 oColour;

void main()
{
	oColour = vec4(vec3(0.9, 0.61, 0.42) * varTimeToLive, 1.0);
}
