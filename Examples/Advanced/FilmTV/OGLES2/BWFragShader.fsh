uniform sampler2D  sTexture;

varying lowp    float  LightIntensity;
varying mediump vec2   TexCoord;

void main()
{
	lowp vec3 fCol = texture2D(sTexture, TexCoord).rgb;
	lowp float fAvg = (fCol.r + fCol.g + fCol.b) / 3.0;
    gl_FragColor.rgb = vec3(fAvg * LightIntensity);
    gl_FragColor.a = 1.0;
}
