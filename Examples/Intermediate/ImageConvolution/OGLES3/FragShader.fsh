#version 300 es
uniform sampler2D    sTexture;

in highp vec2 TexCoord;

out highp vec4 fragColor;

void main()
{
    fragColor = texture(sTexture, TexCoord);    
}
