attribute highp   vec2  inVertex;
attribute mediump vec2  inTexCoord;

varying mediump vec2   TexCoord;

void main()
{
    // Pass through vertex
	gl_Position = vec4(inVertex, 0.0, 1.0);
	
	// Pass through texcoords
	TexCoord = inTexCoord;
}
