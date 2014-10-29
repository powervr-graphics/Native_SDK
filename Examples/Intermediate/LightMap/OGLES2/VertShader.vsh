attribute highp   vec3  inVertex;
attribute mediump vec3  inNormal;
attribute mediump vec2  inTexCoord;

uniform highp   mat4  MVPMatrix;
uniform highp   mat4  ShadowProj;
uniform mediump vec3  LightDirModel;
uniform mediump vec3  EyePosModel;
uniform mediump mat3  ModelWorld;

varying mediump vec2   TexCoord;
varying mediump vec3   ShadowCoord;
varying mediump vec2   ReflectCoord;
varying lowp    float  LightIntensity;

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
		
	// Simple diffuse lighting
	LightIntensity = max(dot(inNormal, LightDirModel), 0.0);
	
	// Calculate eye direction in model space
	mediump vec3 eyeDir = normalize(inVertex - EyePosModel);
	
	// reflect eye direction over normal and transform to world space
	ReflectCoord = vec2(ModelWorld * reflect(eyeDir, inNormal)) * 0.5 + 0.5;
	
	ShadowCoord = (ShadowProj * vec4(inVertex, 1.0)).xyw;
	ShadowCoord.xy += ShadowCoord.z;
	ShadowCoord.z *= 2.0;
	
	// Pass through texcoords
	TexCoord = inTexCoord;
}