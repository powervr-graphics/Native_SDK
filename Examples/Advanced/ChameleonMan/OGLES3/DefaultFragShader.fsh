#version 300 es

uniform sampler2D sTexture;

in mediump vec2 TexCoord;

layout (location = 0) out lowp vec4 oColour;

void main()
{
    oColour = texture(sTexture, TexCoord);
}
