#version 300 es
uniform lowp sampler2D  sTexture;

/* 
  Separated Gaussian 5x5 filter, first row:

              1  5  6  5  1
*/

in mediump vec2  TexCoord0;
in mediump vec2  TexCoord1;
in mediump vec2  TexCoord2;

layout (location = 0) out lowp vec4 oColor;

void main()
{
    lowp vec3 color = texture(sTexture, TexCoord0).rgb * 0.333333;
    color = color + texture(sTexture, TexCoord1).rgb * 0.333333;
    color = color + texture(sTexture, TexCoord2).rgb * 0.333333;    

    oColor.rgb = color;
	oColor.a = 1.;
}
