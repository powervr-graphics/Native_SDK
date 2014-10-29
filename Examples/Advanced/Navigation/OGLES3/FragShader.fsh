#version 300 es

uniform lowp vec4 FlatColour;
layout (location = 0) out lowp vec4 oColour;

void main()
{	
	oColour = FlatColour;
}
