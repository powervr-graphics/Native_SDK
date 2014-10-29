uniform lowp sampler2D  sTexture;

/* 
  Separated Gaussian 5x5 filter, first row:

              1  5  6  5  1
*/

varying mediump vec2  TexCoord0;
varying mediump vec2  TexCoord1;
varying mediump vec2  TexCoord2;

void main()
{
    lowp vec3 color = texture2D(sTexture, TexCoord0).rgb * 0.333333;
    color = color + texture2D(sTexture, TexCoord1).rgb * 0.333333;
    color = color + texture2D(sTexture, TexCoord2).rgb * 0.333333;    

    gl_FragColor.rgb = color;
}
