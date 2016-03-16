#version 440
layout(set = 0, binding = 0)uniform sampler2D  sTexture;
	
layout (location = 0) in lowp    float  LightIntensity;
layout (location = 1) in mediump vec2   TexCoord;
layout (location = 0) out lowp vec4 oColor;

void main()
{
	oColor.rgb = texture(sTexture, TexCoord).rgb * LightIntensity;
	oColor.a = 1.0;
}