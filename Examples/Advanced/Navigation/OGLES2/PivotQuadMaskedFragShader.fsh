uniform lowp    sampler2D  sTexture;
uniform lowp    vec4       Colour;

varying mediump vec2       TexCoord;

void main()
{ 
	// Write the constant colour and modulate the alpha with the intensity value from the texture   
	gl_FragColor = vec4(Colour.rgb, Colour.a * texture2D(sTexture, TexCoord).r);
}
