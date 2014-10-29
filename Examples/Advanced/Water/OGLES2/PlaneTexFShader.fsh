uniform sampler2D		Texture;

uniform mediump vec2 	RcpWindowSize;

void main()
{	
	mediump vec2 vTexCoord = gl_FragCoord.xy * RcpWindowSize;
	gl_FragColor = texture2D(Texture, vTexCoord);
}
