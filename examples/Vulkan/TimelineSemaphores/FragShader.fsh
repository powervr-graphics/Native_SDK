#version 320 es

layout (set = 0, binding = 0) uniform mediump sampler2D sBaseTex;

layout(location = 0) in mediump vec2 TexCoord;

layout(location = 0) out mediump vec4 oColor;

void main()
{
	mediump float texColor = texture(sBaseTex, TexCoord).r;
	oColor = vec4(vec3(texColor), 1.0);
}
