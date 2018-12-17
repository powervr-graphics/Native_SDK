attribute highp vec2 inVertex;

varying mediump vec2 vTexCoord;
uniform highp mat4 uvTransform;

void main()
{
	mediump vec2 madd = vec2(.5,.5);
	gl_Position = vec4(inVertex,0.,1.);
	vTexCoord = (uvTransform * vec4(inVertex * madd + madd ,0.,1.)).xy;
}
