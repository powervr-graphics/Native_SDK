#version 300 es

uniform sampler2D  sTexture;

in mediump vec2 TexCoord;
in lowp    vec4 LightColour;

layout (location = 0) out lowp vec4 oColour;

void main()
{
    oColour = LightColour * texture(sTexture, TexCoord);
}
