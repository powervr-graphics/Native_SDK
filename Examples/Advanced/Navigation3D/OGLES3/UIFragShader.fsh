#version 300 es

uniform lowp sampler2D sTexture;
uniform lowp vec4      ColourScale;
 
in mediump vec2 vTexCoord;
layout (location = 0) out lowp vec4 oColour;

void main()
{	
	oColour = texture(sTexture, vTexCoord) * ColourScale;
}
