#version 300 es
uniform sampler2D  sTexture;

in lowp    float LightIntensity;
in mediump vec2  TexCoord;

layout (location = 0) out lowp vec4 oColour;

void main()
{
	lowp vec3 fCol = texture(sTexture, TexCoord).rgb;
	lowp float fAvg = (fCol.r + fCol.g + fCol.b) / 3.0;
    oColour.rgb = vec3(fAvg * LightIntensity);
    oColour.a = 1.0;
}
