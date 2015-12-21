attribute highp   vec3 inVertex;
attribute highp   vec3	inNormal;
attribute mediump vec2 inTexCoord;

uniform highp   mat4 ModelMatrix;
uniform highp   mat4 MVPMatrix;
uniform highp   mat3 ModelWorldIT3x3;
uniform highp	vec3 LightPos;

varying mediump vec2  TexCoord;
varying mediump vec3 vWorldNormal;
varying mediump vec3 vLightDir;
varying mediump float vOneOverAttenuation;

void main()
{
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);

	vec3 worldPos = (ModelMatrix * vec4(inVertex, 1.0)).xyz;
	vLightDir = LightPos - worldPos;
	float light_distance = length(vLightDir);
	vLightDir /= light_distance;

	vOneOverAttenuation = 1. / (1. + .00005*light_distance*light_distance);

	vWorldNormal = ModelWorldIT3x3 * inNormal;
	// Pass through texcoords
	TexCoord = inTexCoord;
}
 