uniform lowp sampler2D sTexture;
uniform lowp vec4   FlatColour;

varying highp vec2  vTexCoord;
varying highp float vDiffuse;


void main()
{	
	lowp vec4 colour = texture2D(sTexture, vTexCoord);
	gl_FragColor.rgb = colour.rgb * vDiffuse;	
	gl_FragColor.a = colour.a;	
}
