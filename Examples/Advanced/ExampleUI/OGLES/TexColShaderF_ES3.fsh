#version 300 es
uniform sampler2D Texture;
uniform lowp vec4 vRGBA;

in highp vec2 TexCoord;

layout (location = 0) out lowp vec4 oColor;

void main()
{
	oColor = texture(Texture, TexCoord) * vRGBA;
}