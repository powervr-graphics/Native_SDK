#version 300 es
uniform sampler2D sTexture;
uniform mediump vec3 LightDirection;

in mediump vec2 TexCoord;
in mediump vec3 worldNormal;

out mediump vec4 oColor;

void main()
{
	// Simple diffuse lighting in model space
	highp float LightIntensity = max(dot(worldNormal, -LightDirection), 0.0);

    oColor.rgb = texture(sTexture, TexCoord).rgb * LightIntensity;
    oColor.a = 1.0;
}
