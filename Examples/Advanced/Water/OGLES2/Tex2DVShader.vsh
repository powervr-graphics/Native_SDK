attribute highp vec3 inVertex;
attribute highp vec2 inTexCoord;


uniform mediump mat4 MVPMatrix;

varying highp vec2 TexCoord;

void main()
{
	TexCoord = inTexCoord;
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
}