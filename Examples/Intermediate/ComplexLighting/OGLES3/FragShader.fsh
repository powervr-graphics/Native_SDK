#version 300 es
uniform sampler2D  sTexture;

in mediump vec2  TexCoord;
in lowp    vec3  DiffuseLight;
in lowp    vec3  SpecularLight;

layout (location = 0) out lowp vec4 oColour;

void main()
{
    lowp vec3 texColor  = vec3(texture(sTexture, TexCoord));
	lowp vec3 color = (texColor * DiffuseLight) + SpecularLight;
	oColour = vec4(color, 1.0);
}
