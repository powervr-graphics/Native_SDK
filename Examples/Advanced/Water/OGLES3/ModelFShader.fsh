#version 300 es
#define ENABLE_TEXTURE
#ifdef ENABLE_TEXTURE
uniform sampler2D		ModelTexture;
#ifdef ENABLE_SPECULAR
uniform sampler2D		ModelTextureSpec;
#endif
#endif

#ifdef ENABLE_FOG_DEPTH
uniform lowp vec3 		FogColour;
uniform mediump float 	RcpMaxFogDepth;
#endif

#ifdef ENABLE_LIGHTING
    uniform lowp vec3       EmissiveColour;
    uniform lowp vec3       DiffuseColour;
	in lowp float			LightIntensity;
	#ifdef ENABLE_SPECULAR
        uniform lowp vec3       SpecularColour;
        in mediump vec3    EyeDir;
        in mediump vec3    LightDir;
	    in mediump vec3    Normal;
    #endif
#endif
#ifdef ENABLE_TEXTURE
	in mediump vec2   	TexCoord;
#endif
#ifdef ENABLE_FOG_DEPTH
	in mediump float 	VertexDepth;
#endif

layout (location = 0) out lowp vec4 oColour;

void main()
{	
	#ifdef ONLY_ALPHA
		oColour = vec4(vec3(0.5),0.0);
	#else
		#ifdef ENABLE_TEXTURE
			#ifdef ENABLE_FOG_DEPTH		
				// Mix the object's colour with the fogging colour based on fragment's depth
				lowp vec3 vFragColour = texture(ModelTexture, TexCoord).rgb;
				
				// Perform depth test and clamp the values
				lowp float fFogBlend = clamp(VertexDepth * RcpMaxFogDepth, 0.0, 1.0);
				
				#ifdef ENABLE_LIGHTING
					oColour.rgb = mix(vFragColour.rgb * LightIntensity, FogColour.rgb, fFogBlend);
				#else
					oColour.rgb = mix(vFragColour.rgb, FogColour.rgb, fFogBlend);
				#endif
                    oColour.a = 1.0;
			#else
				#ifdef ENABLE_LIGHTING
					lowp vec3 vSpec = vec3(0.0);
                    #ifdef ENABLE_SPECULAR
	                    lowp vec3 N        = normalize(Normal);
	                    lowp vec3 refl     = reflect(-LightDir, N);
	                    lowp float SpecPow = clamp(dot(-EyeDir, refl), 0.0, 1.0);
	                    SpecPow = pow(SpecPow, 4.0);
                      	vSpec = SpecularColour * texture(ModelTextureSpec, TexCoord).rgb * SpecPow;
					#endif
	                lowp vec4 vTex    = texture(ModelTexture, TexCoord);
	                lowp vec3 vDiff   = vTex.rgb * DiffuseColour * max(EmissiveColour, LightIntensity);
					oColour = vec4(vDiff + vSpec, vTex.a);
				#else
					oColour = vec4(texture(ModelTexture, TexCoord).rgb, 1.0);
				#endif
			#endif
		#else
			// Solid colour is used instead of texture colour
			#ifdef ENABLE_LIGHTING
				oColour = vec4(vec3(0.3,0.3,0.3)* LightIntensity, 1.0);
			#else
				oColour = vec4(vec3(0.3,0.3,0.3), 1.0);	
			#endif
		#endif
	#endif
}
