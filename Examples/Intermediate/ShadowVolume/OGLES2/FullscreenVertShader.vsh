attribute highp vec2  inVertex;

void main()
{
	gl_Position = vec4(inVertex, 0.0, 1.0);
}
