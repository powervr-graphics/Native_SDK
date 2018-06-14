#version 300 es
uniform sampler2D  sTexture;

in highp    float       LightIntensity;
in mediump vec2         TexCoord;

layout (location = 0) out highp vec4 oColor;

void main()
{
    oColor = texture(sTexture, TexCoord) * LightIntensity;
}
