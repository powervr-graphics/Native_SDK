#version 100

uniform sampler2D sParaboloids;
uniform samplerCube sSkybox;

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
	mediump vec3 Normalised;

#ifdef REFLECT
	// Sample reflection to skybox
	lowp vec4 ReflectSky = textureCube(sSkybox, ReflectDir);

	// Sample reflection to paraboloids
	lowp vec4 Reflection;

	Normalised = normalize(ReflectDir);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	Reflection = texture2D(sParaboloids, Normalised.xy);

	// Combine skybox reflection with paraboloid reflection
	Reflection.rgb = mix(ReflectSky.rgb, Reflection.rgb, Reflection.a);
#endif

#ifdef REFRACT
#ifdef CHROMATIC
	// Sample refraction to skybox
	lowp vec4 RefractSky;
	RefractSky.r = textureCube(sSkybox, RefractDirRed).r;
	RefractSky.g = textureCube(sSkybox, RefractDirGreen).g;
	RefractSky.b = textureCube(sSkybox, RefractDirBlue).b;
	
	// Sample refraction to paraboloids
	lowp vec4 Refraction;

	// Red
	Normalised = normalize(RefractDirRed);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	lowp vec4 RefractRed = texture2D(sParaboloids, Normalised.xy);

	Refraction.r = mix(RefractSky.r, RefractRed.r, RefractRed.a);

	// Green
	Normalised = normalize(RefractDirGreen);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	lowp vec4 RefractGreen = texture2D(sParaboloids, Normalised.xy);

	Refraction.g = mix(RefractSky.g, RefractGreen.g, RefractGreen.a);

	// Blue
	Normalised = normalize(RefractDirBlue);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	lowp vec4 RefractBlue = texture2D(sParaboloids, Normalised.xy);

	Refraction.b = mix(RefractSky.b, RefractBlue.b, RefractBlue.a);
#else
	lowp vec4 RefractSky = textureCube(sSkybox, RefractDir);

	Normalised = normalize(RefractDir);
	Normalised.xy /= abs(Normalised.z) + 1.0;
	Normalised.xy = Normalised.xy * 0.495 + 0.5;
	Normalised.x *= 0.5;
	Normalised.x += sign(-Normalised.z) * 0.25 + 0.25;
	lowp vec4 Refraction = texture2D(sParaboloids, Normalised.xy);

	Refraction.rgb = mix(RefractSky.rgb, Refraction.rgb, Refraction.a);
#endif
#endif

#if defined(REFLECT) && defined(REFRACT)
	// Combine reflection and refraction for final colour
	gl_FragColor.rgb = mix(Refraction.rgb, Reflection.rgb, ReflectFactor);
#elif defined(REFLECT)
	gl_FragColor.rgb = Reflection.rgb;
#elif defined(REFRACT)
	gl_FragColor.rgb = Refraction.rgb;
#endif
}
