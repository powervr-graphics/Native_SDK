attribute highp   vec3  inVertex;
attribute mediump vec3  inNormal;
attribute mediump vec2  inTexCoord;

uniform highp   mat4    MVPMatrix;
#ifdef DIFFUSE
uniform mediump vec3    vLightPosition;
varying lowp    float   fLightIntensity;
#endif

varying mediump vec2    vTexCoord;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);

	// Pass through texcoords
	vTexCoord = inTexCoord;

#ifdef DIFFUSE
	// Simple diffuse lighting in model space
	mediump vec3 vLightDir = normalize(vLightPosition - inVertex);
	fLightIntensity = dot(normalize(inNormal), vLightDir);
#endif
}