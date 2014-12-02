#version 300 es
uniform sampler2D  sTexture;

in lowp    float  SpecularIntensity;
in mediump vec2   RefractCoord;

layout (location = 0) out lowp vec4 oColour;

void main()
{
	lowp vec3 refractColor = texture(sTexture, RefractCoord).rgb;	
	oColour = vec4(refractColor + SpecularIntensity, 1.0);
}