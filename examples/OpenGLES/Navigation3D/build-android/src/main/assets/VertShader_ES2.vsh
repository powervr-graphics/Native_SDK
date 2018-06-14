attribute highp vec3	myVertex;

uniform highp mat4 transform;
uniform lowp vec4 myColour;

varying lowp vec4 fragColour;

void main(void)
{
	gl_Position = transform * vec4(myVertex, 1.0);
	fragColour = myColour;
}