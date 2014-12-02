#version 300 es
uniform lowp sampler2D sTexture;
uniform lowp vec4   FlatColour;

in highp vec2  vTexCoord;
in highp float vDiffuse;

layout (location = 0) out lowp vec4 oColour;

void main()
{	
	lowp vec4 colour = texture(sTexture, vTexCoord);
	oColour.rgb = colour.rgb * vDiffuse;	
	oColour.a = colour.a;	
}
