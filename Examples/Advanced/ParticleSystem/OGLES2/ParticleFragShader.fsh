uniform sampler2D sTexture;

varying mediump vec2 vTexCoord;

void main()
{
	gl_FragColor = vec4(texture2D(sTexture, vTexCoord).rgb, vTexCoord.s);
}
