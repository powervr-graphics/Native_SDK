
uniform sampler2D  sTexture;
uniform sampler2D  sBlurTexture;
uniform mediump float sTexFactor;
uniform mediump float sBlurTexFactor;

varying mediump vec2 TexCoord;
void main()
{
    gl_FragColor = (texture2D(sTexture, TexCoord) * sTexFactor) + (texture2D(sBlurTexture, TexCoord) * sBlurTexFactor);
}
