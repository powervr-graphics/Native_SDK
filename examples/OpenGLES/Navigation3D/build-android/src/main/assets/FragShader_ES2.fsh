varying mediump vec4 fragColour;

void main(void)
{
	gl_FragColor = vec4(fragColour.rgb, 1.0);
}