attribute highp   vec2  inVertex;

uniform highp mat4    TexSamplerPMatrix;

varying mediump vec2   sTexCoord;

void main()
{
	gl_Position = vec4(inVertex,0.0,1.0 );	
	vec2 madd = vec2(.5,.5);
	vec2 inTexCoord = inVertex.xy * madd + madd;
	// TexCoordinates modified by Android Camera Projection Matrix
	sTexCoord    = (TexSamplerPMatrix * vec4(inTexCoord, 0, 1)).xy;
}
