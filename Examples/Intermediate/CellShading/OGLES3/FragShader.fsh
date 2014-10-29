#version 300 es
uniform sampler2D  sTexture;

in mediump vec2  TexCoord;

const lowp vec3  cBaseColor = vec3(0.2, 1.0, 0.7);

layout (location = 0) out lowp vec4 oColour;

void main()
{
	oColour = vec4(cBaseColor * texture(sTexture, TexCoord).rgb, 1.0);
}

