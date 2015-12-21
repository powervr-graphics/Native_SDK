#version 300 es

in highp vec3	inVertex;
in highp vec3	inNormal;
in mediump vec2	inTexCoord;

uniform highp   mat4 ModelMatrix;
uniform highp   mat4 MVPMatrix;
uniform highp   mat3 ModelWorldIT3x3;
uniform highp	vec3 LightPos;

out mediump vec2 vTexCoord;
out mediump vec3 vWorldNormal;
out mediump vec3 vLightDir;
out mediump float vOneOverAttenuation;


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
	vTexCoord = inTexCoord;
}
 