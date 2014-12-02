uniform sampler2D  sTexture;

//varying lowp    float  LightIntensity;
varying mediump vec2   TexCoord;
varying lowp    vec4  LightColour;

void main()
{
    gl_FragColor = LightColour * texture2D(sTexture, TexCoord);
}
