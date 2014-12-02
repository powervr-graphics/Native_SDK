uniform samplerCube CubeMap;

uniform lowp vec4 FogColour;
uniform mediump float RcpMaxFogDepth;

#ifdef ENABLE_DISCARD_CLIP
uniform bool ClipPlaneBool;
#endif
varying mediump vec3 EyeDir;
varying mediump float VertexHeight;
#ifdef ENABLE_DISCARD_CLIP
varying highp float ClipDist;
#endif

void main()
{
	#ifdef ENABLE_DISCARD_CLIP
		// Reject fragments behind the clip plane
		if(ClipDist < 0.0)
		{
			discard; // Too slow for hardware. Left as an example of how not to do this!
		}
	#endif
	
	// Mix the object's colour with the fogging colour based on fragment's depth
	lowp vec3 vFragColour = textureCube(CubeMap, EyeDir).rgb;
	
	// Test depth
	lowp float fFogBlend = 1.0 - clamp(VertexHeight * RcpMaxFogDepth, 0.0, 1.0);
	vFragColour.rgb = mix(vFragColour.rgb, FogColour.rgb, fFogBlend);
		
	gl_FragColor = vec4(vFragColour.rgb, 1.0);
}