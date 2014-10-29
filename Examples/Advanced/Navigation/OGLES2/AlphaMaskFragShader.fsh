uniform mediump sampler2D  sTexture;
uniform lowp    vec4       FlatColour;

varying mediump vec2    TexCoord;

void main()
{	
	gl_FragColor = vec4(FlatColour.rgb, FlatColour.a * texture2D(sTexture, TexCoord).a);		
}
