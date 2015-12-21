attribute highp   vec2  inVertex;
attribute mediump vec2  inTexCoord;

varying mediump vec2   TexCoord;

uniform mediump mat4 MVPMatrix;

void main()
{
    // Pass through vertex
	gl_Position = MVPMatrix * vec4(inVertex, 0.0, 1.0);
	
	// Pass through texcoords
	TexCoord = inTexCoord;
}
