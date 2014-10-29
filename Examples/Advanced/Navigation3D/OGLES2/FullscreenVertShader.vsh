attribute highp vec2  inVertex;
uniform   lowp  vec4  FlatColour;
varying   lowp  vec4  vColour;


void main()
{
	gl_Position = vec4(inVertex, 0.0, 1.0);
	vColour = FlatColour;
}
