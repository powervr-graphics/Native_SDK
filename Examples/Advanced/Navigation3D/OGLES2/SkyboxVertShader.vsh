attribute mediump vec3 inVertex;

uniform mediump mat4 ModelViewProjMatrix;

varying mediump vec3 vEyeDir;

void main()
{	
	vEyeDir = -inVertex.xzy;
	gl_Position = ModelViewProjMatrix * vec4(inVertex, 1.0);	
}