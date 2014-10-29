uniform mediump sampler2D  sTexture;
uniform lowp    vec4       FlatColour;

varying mediump vec2    TexCoord;

void main()
{
	lowp vec4 texcol = texture2D(sTexture, TexCoord);
	gl_FragColor = vec4(FlatColour.rgb * texcol.r, texcol.a);
}
