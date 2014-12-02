uniform sampler2D  sTexture;

varying mediump vec2   TexCoord;
varying lowp vec4   Colors;

void main()
{
    gl_FragColor = texture2D(sTexture, TexCoord) * Colors;
}
