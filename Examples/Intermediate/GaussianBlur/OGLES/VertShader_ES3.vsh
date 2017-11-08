#version 310 es

in vec3 inPosition;
in vec2 inTexCoord;

out mediump vec2 TexCoord;

void main()
{
	gl_Position = vec4(inPosition, 1.0);
	TexCoord = inTexCoord;
}