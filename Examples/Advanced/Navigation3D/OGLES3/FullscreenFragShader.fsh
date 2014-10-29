#version 300 es

in lowp vec4 vColour;

layout (location = 0) out lowp vec4 oColour;

void main()
{	
	oColour = vColour;
}
