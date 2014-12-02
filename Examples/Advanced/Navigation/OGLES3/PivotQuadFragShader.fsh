#version 300 es

uniform lowp    sampler2D  sTexture;
uniform lowp    vec4       Colour;

in mediump vec2       TexCoord;

layout (location = 0) out lowp vec4 oColour;

void main()
{    
	// Multiply the texture colour with the constant colour
	oColour = texture(sTexture, TexCoord) * Colour;	
}
