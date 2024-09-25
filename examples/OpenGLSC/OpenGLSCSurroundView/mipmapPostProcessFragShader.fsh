#version 310 es

uniform mediump sampler2D mipTexture;

in mediump vec2 UV;

layout (location = 0) out mediump vec4 oColor;

void main()
{
	oColor = texture(mipTexture, UV);
	oColor.x = 1.0;
	oColor.y = 1.0;
	oColor.z = 1.0;
}