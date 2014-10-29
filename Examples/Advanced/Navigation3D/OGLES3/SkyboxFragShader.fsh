#version 300 es

uniform samplerCube sCubeMap;

in mediump vec3 vEyeDir;
layout (location = 0) out lowp vec4 oColour;

void main()
{
	oColour = texture(sCubeMap, vEyeDir);
}