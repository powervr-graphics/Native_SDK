#version 300 es

/*
  Simple fragment shader:
  - Single texturing modulated with vertex lighting
*/

uniform sampler2D sTexture;

in lowp    float LightIntensity;
in mediump vec2  TexCoord;

layout (location = 0) out lowp vec4 oColour;

void main()
{
    oColour = vec4(texture(sTexture, TexCoord).rgb * LightIntensity, 1.0);
}
