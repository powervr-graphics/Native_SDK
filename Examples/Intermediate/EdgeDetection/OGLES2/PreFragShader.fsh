uniform lowp vec4 inColor;	// Color and ID passed in from vertex.
void main()
{
	// Simply assigns the color and ID number of the object it renders.
	gl_FragColor = inColor;
}

