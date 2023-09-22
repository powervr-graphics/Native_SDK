attribute highp vec4	myVertex;
uniform highp mat4 transformationMatrix;
void main(void)
{
	gl_Position = transformationMatrix * myVertex;
}