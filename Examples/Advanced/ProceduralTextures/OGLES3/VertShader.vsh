attribute vec2 inVertex;
attribute vec2 inTexCoord;

varying mediump vec2 vTexCoord;

void main()
{
	// Pass through position
	gl_Position = vec4(inVertex, 0.0, 1.0);	

	// Pass through texcoords
	vTexCoord = inTexCoord;	
}