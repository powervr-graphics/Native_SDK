attribute highp   vec3  inVertex;
attribute mediump vec3  inNormal;

uniform highp   mat4  MVPMatrix;
uniform mediump mat3  ModelWorld;
uniform mediump vec3  EyePosModel;

varying mediump vec3  ReflectDir;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	
	// Calculate eye direction in model space
	mediump vec3 eyeDir = normalize(inVertex - EyePosModel);
	
	// reflect eye direction over normal and transform to world space
	ReflectDir = ModelWorld * reflect(eyeDir, inNormal);
}