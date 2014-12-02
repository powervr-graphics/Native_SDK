attribute highp vec3 inVertex;		// Vertex coordinates
uniform highp   mat4 MVPMatrix;		// Model/View/Position matrix

void main()
{
	// Assign and transform position of the vertex so it is viewed from the correct angle.
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
}