#version 300 es
in highp vec3  inVertex;
in highp vec2  inTexCoord;

out highp vec2   TexCoord;

void main()
{
	// Pass through position
	gl_Position = vec4(inVertex, 1.0);	

	// Pass through texcoords
	TexCoord = inTexCoord;	
}