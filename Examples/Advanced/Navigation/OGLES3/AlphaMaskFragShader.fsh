#version 300 es

uniform mediump sampler2D  sTexture;
uniform lowp    vec4       FlatColour;

in mediump vec2    TexCoord;

layout (location = 0) out lowp vec4 oColour;

void main()
{	
	oColour = vec4(FlatColour.rgb, FlatColour.a * texture(sTexture, TexCoord).a);		
}
