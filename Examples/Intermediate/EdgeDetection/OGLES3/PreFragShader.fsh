#version 300 es

uniform lowp vec4 inColor;	// Colour and ID passed in from vertex.
layout (location = 0) out lowp vec4 oColour;

void main()
{
	// Simply assigns the colour and ID number of the object it renders.
	oColour = inColor;
}

