#version 300 es

uniform sampler2D  sBasetex;
uniform sampler2D  sReflect;
uniform sampler2D  sShadow;

in mediump vec2   TexCoord;
in mediump vec3   ShadowCoord;
in mediump vec2   ReflectCoord;
in lowp    float  LightIntensity;

const lowp float  cReflect = 0.2;

layout (location = 0) out lowp vec4 oColour;

void main()
{
	lowp vec3 baseColour = texture(sBasetex, TexCoord).rgb;
	baseColour *= 0.2 + 0.8 * textureProj(sShadow, ShadowCoord).r * LightIntensity;
	
	lowp vec3 reflectColour = texture(sReflect, ReflectCoord).rgb;

	oColour = vec4(baseColour +  reflectColour * cReflect, 1.0);
}