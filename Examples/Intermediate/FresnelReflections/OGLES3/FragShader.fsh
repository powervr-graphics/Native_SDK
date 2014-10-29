#version 300 es

uniform sampler2D  sBaseTex;
uniform sampler2D  sReflectTex;

in mediump vec2   ReflectCoord;
in mediump vec2   TexCoord;
in lowp    float  ReflectRatio;

layout (location = 0) out lowp vec4 oColour;

void main()
{
	lowp vec3 baseColour = vec3(texture(sBaseTex, TexCoord));
	lowp vec3 reflection = vec3(texture(sReflectTex, ReflectCoord));
	lowp vec3 colour = mix(baseColour, reflection, ReflectRatio);
	oColour = vec4(colour, 1.0);
}
