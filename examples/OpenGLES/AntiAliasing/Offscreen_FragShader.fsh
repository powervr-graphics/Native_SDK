#version 310 es

uniform sampler2D screenTexture;

layout(location = 0) in mediump vec2 TexCoords;

out mediump vec4 oColor;

void main()
{
	mediump vec3 currentColor = texture(screenTexture, TexCoords).xyz;
	oColor = vec4(currentColor, 1.0f);
}
