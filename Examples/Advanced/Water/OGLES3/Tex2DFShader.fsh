#version 300 es
uniform sampler2D Texture;

in highp vec2 TexCoord;

layout (location = 0) out lowp vec4 oColour;

void main()
{
	oColour = texture(Texture, TexCoord);
}