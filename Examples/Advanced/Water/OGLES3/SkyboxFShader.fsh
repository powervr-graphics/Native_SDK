#version 300 es
uniform samplerCube CubeMap;

uniform lowp vec4 FogColour;
uniform mediump float RcpMaxFogDepth;

#ifdef ENABLE_DISCARD_CLIP
uniform bool ClipPlaneBool;
#endif
in mediump vec3 EyeDir;
in mediump float VertexHeight;
#ifdef ENABLE_DISCARD_CLIP
in highp float ClipDist;
#endif

layout (location = 0) out lowp vec4 oColour;

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
	lowp vec3 vFragColour = texture(CubeMap, EyeDir).rgb;
		
	// Test depth
	lowp float fFogBlend = 1.0 - clamp(VertexHeight * RcpMaxFogDepth, 0.0, 1.0);
	vFragColour.rgb = mix(vFragColour.rgb, FogColour.rgb, fFogBlend);
			
	oColour = vec4(vFragColour.rgb, 1.0);
}
