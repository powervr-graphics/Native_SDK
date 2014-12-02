#version 300 es

layout (location = 0) in highp vec3	inVertex;
layout (location = 1) in mediump vec3	inNormal;
layout (location = 2) in mediump vec2	inTexCoord;

uniform highp   mat4  MVPMatrix;
uniform highp   mat4  ShadowProj;
uniform mediump vec3  LightDirModel;
uniform mediump vec3  EyePosModel;
uniform mediump mat3  ModelWorld;

out mediump vec2   TexCoord;
out mediump vec3   ShadowCoord;
out mediump vec2   ReflectCoord;
out lowp    float  LightIntensity;

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