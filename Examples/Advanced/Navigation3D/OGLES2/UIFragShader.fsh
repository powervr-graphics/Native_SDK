uniform lowp sampler2D sTexture;
uniform lowp vec4      ColourScale;
 
varying mediump vec2 vTexCoord;


void main()
{	
	gl_FragColor = texture2D(sTexture, vTexCoord) * ColourScale;
}
