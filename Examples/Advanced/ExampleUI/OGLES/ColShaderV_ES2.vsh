attribute highp vec3 inVertex;

uniform mediump mat4 MVPMatrix;

void main()
{
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
}