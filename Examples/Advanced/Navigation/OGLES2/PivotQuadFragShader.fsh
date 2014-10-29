uniform lowp    sampler2D  sTexture;
uniform lowp    vec4       Colour;

varying mediump vec2       TexCoord;

void main()
{    
	// Multiply the texture colour with the constant colour
	gl_FragColor = texture2D(sTexture, TexCoord) * Colour;	
}
