#version 300 es

uniform lowp    sampler2D  sTexture;
uniform lowp    vec4       Colour;

in mediump vec2       TexCoord;

layout (location = 0) out lowp vec4 oColour;

void main()
{ 
	// Write the constant colour and modulate the alpha with the intensity value from the texture   
	oColour = vec4(Colour.rgb, Colour.a * texture(sTexture, TexCoord).r);
}
