#version 300 es

uniform sampler2D  sTexture;

in highp   float  LightIntensity;
in mediump vec2   TexCoord;

layout(location = 0) out lowp vec4 oColour;

void main()
{
    oColour.rgb = texture(sTexture, TexCoord).rgb * LightIntensity;
	  oColour.a = 1.0;
}
