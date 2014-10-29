#version 100

#ifdef REFRACT
#ifdef CHROMATIC
const lowp vec3 Eta = vec3(0.85, 0.87, 0.89);
#else
const lowp float Eta = 0.87;
#endif
#endif

#if defined(REFLECT) && defined(REFRACT)
const lowp float FresnelBias = 0.3;
const lowp float FresnelScale = 0.7;
const lowp float FresnelPower = 1.5;
#endif

uniform highp mat4 MVPMatrix;
uniform mediump mat3 MMatrix;
uniform mediump vec3 EyePos;

attribute highp vec3 inVertex;
attribute mediump vec3 inNormal;

#ifdef REFLECT
varying mediump vec3 ReflectDir;
#endif

#ifdef REFRACT
#ifdef CHROMATIC
varying mediump vec3 RefractDirRed;
varying mediump vec3 RefractDirGreen;
varying mediump vec3 RefractDirBlue;
#else
varying mediump vec3 RefractDir;
#endif
#endif

#if defined(REFLECT) && defined(REFRACT)
varying highp float ReflectFactor;
#endif

void main()
{
	// Transform position
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	
	// Calculate view direction in model space
	mediump vec3 ViewDir = normalize(inVertex - EyePos);

#ifdef REFLECT	
	// Reflect view direction and transform to world space
	ReflectDir = MMatrix * reflect(ViewDir, inNormal);
#endif

#ifdef REFRACT
#ifdef CHROMATIC
	// Refract view direction and transform to world space
	RefractDirRed = MMatrix * refract(ViewDir, inNormal, Eta.r);
	RefractDirGreen = MMatrix * refract(ViewDir, inNormal, Eta.g);
	RefractDirBlue = MMatrix * refract(ViewDir, inNormal, Eta.b);
#else
	RefractDir = MMatrix * refract(ViewDir, inNormal, Eta);
#endif
#endif

#if defined(REFLECT) && defined(REFRACT)
	// Calculate the reflection factor
	ReflectFactor = FresnelBias + FresnelScale * pow(1.0 + dot(ViewDir, inNormal), FresnelPower);
	ReflectFactor = clamp(ReflectFactor, 0.0, 1.0);
#endif
}
