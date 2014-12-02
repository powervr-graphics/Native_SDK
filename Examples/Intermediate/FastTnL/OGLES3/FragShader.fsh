#version 300 es

uniform sampler2D sTexture;

in lowp    vec3  DiffuseLight;
in lowp    vec3  SpecularLight;
in mediump vec2  TexCoord;

layout (location = 0) out lowp vec4 oColour;

void main()
{
	lowp vec3 texColor  = texture(sTexture, TexCoord).rgb;
	lowp vec3 color = (texColor * DiffuseLight) + SpecularLight;
	oColour = vec4(color, 1.0);
}

