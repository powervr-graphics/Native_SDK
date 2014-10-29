attribute highp vec3 inVertex;
attribute highp vec2 inUVs;

uniform mediump mat4 MVPMatrix;

varying highp vec2 TexCoord;

void main()
{
	TexCoord = inUVs;
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
}