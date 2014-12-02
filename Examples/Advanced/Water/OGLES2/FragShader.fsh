uniform sampler2D		NormalTex;
uniform sampler2D		ReflectionTex;
#ifdef ENABLE_REFRACTION
	uniform sampler2D		RefractionTex;
#endif
uniform samplerCube NormalisationCubeMap;

uniform mediump mat4	ModelViewMatrix;
uniform lowp vec4		WaterColour;
uniform mediump float   RcpMaxFogDepth;
uniform lowp vec4       FogColour;

#ifdef ENABLE_DISTORTION
	uniform mediump float	WaveDistortion;
#endif
uniform mediump vec2 	RcpWindowSize;

varying mediump vec2 	BumpCoord0;
varying mediump vec2 	BumpCoord1;
varying highp   vec3	WaterToEye;
varying mediump float	WaterToEyeLength;

void main()
{	
	// Calculate the tex coords of the fragment (using it's position on the screen), normal map is z-axis major.
	lowp vec3 vAccumulatedNormal = vec3(0.0,0.0,1.0);
	mediump vec2 vTexCoord = gl_FragCoord.xy * RcpWindowSize;

	// Test depth for fog
	lowp float fFogBlend = clamp(WaterToEyeLength * RcpMaxFogDepth, 0.0, 1.0);
	
	#ifdef ENABLE_DISTORTION
		// When distortion is enabled, use the normal map to calculate perturbation
		vAccumulatedNormal = texture2D(NormalTex, BumpCoord0).rgb;
		vAccumulatedNormal += texture2D(NormalTex, BumpCoord1).rgb;
		vAccumulatedNormal -= 1.0; // Same as * 2.0 - 2.0
	
		lowp vec2 vTmp = vAccumulatedNormal.xy;
		/* 	
			Divide by WaterToEyeLength to scale down the distortion
		 	of fragments based on their distance from the camera 
		*/
		vTexCoord.xy -= vTmp * (WaveDistortion / WaterToEyeLength);
	#endif

#ifdef ENABLE_REFRACTION
	lowp vec4 vReflectionColour = texture2D(ReflectionTex, vTexCoord);
	lowp vec4 vRefractionColour = texture2D(RefractionTex, vTexCoord);
	
	#ifdef ENABLE_FRESNEL
		// Calculate the Fresnel term to determine amount of reflection for each fragment
		
		// Use normalisation cube map instead of normalize() - See section 3.3.1 of white paper for more info
		lowp vec3 vWaterToEyeCube = textureCube(NormalisationCubeMap,WaterToEye).rgb * 2.0 - 1.0;

		//Normal map uses z-axis major instead of y-axis major, so we have to swizzle to switch the normal map's z and y axis.
		mediump float fEyeToNormalAngle = clamp(dot(vWaterToEyeCube,vAccumulatedNormal.xzy),0.0,1.0);
		
		mediump float fAirWaterFresnel = 1.0 - fEyeToNormalAngle;
		fAirWaterFresnel = pow(fAirWaterFresnel, 5.0);
		fAirWaterFresnel = (0.98 * fAirWaterFresnel) + 0.02;	// R(0)-1 = ~0.98 , R(0)= ~0.02
		lowp float fTemp = fAirWaterFresnel;
		
		// Blend reflection and refraction
		lowp vec4 vFragColour = mix(vRefractionColour, vReflectionColour, fTemp);
	    gl_FragColor = mix(vFragColour, FogColour, fFogBlend);
	#else
	    lowp vec4 vFragColour = mix(vRefractionColour, vReflectionColour, 0.4);   // Constant mix
	    gl_FragColor = mix(vFragColour, FogColour, fFogBlend);
	#endif
#else
	gl_FragColor = mix(texture2D(ReflectionTex, vTexCoord), FogColour, fFogBlend);					// Reflection only
#endif
}
