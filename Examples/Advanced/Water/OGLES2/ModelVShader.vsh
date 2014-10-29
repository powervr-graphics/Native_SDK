#define ENABLE_TEXTURE
attribute highp vec3 	inVertex;
attribute highp vec3	inNormal;
attribute highp vec2	inTexCoord;

#ifdef ENABLE_PERTURB_VTX
    uniform highp float fTime;
#endif

uniform highp mat4		MVPMatrix;
uniform highp mat4		ModelMatrix;
uniform mediump vec3	LightDirection;
#ifdef ENABLE_FOG_DEPTH
uniform mediump float	WaterHeight;		//Assume water always lies on the y-axis
#endif

#ifdef ENABLE_LIGHTING
	varying lowp float		LightIntensity;

    #ifdef ENABLE_SPECULAR
        uniform mediump vec3    EyePos;

        varying mediump vec3    EyeDir;
        varying mediump vec3    LightDir;
        varying mediump vec3    Normal;
    #endif
#endif
#ifdef ENABLE_TEXTURE
	varying mediump vec2 	TexCoord;
#endif
#ifdef ENABLE_FOG_DEPTH
	varying mediump float	VertexDepth;
#endif

void main()
{
	// Convert each vertex into projection-space and output the value
	highp vec4 vInVertex   = vec4(inVertex, 1.0);
	mediump vec3 vInNormal = vec3(inNormal);
#ifdef ENABLE_PERTURB_VTX
	lowp float fStr      = inTexCoord.x * 0.7;
	mediump float fDroop = 2.0 * inTexCoord.x;
	vInVertex.y += fStr * sin(fTime + vInVertex.x);
	vInVertex.x += fStr * sin(fTime + vInVertex.x);
	vInVertex.z += fDroop*fDroop;
	vInNormal.x += fStr * cos(fTime + vInVertex.x) / 2.0;
    vInNormal.z += fStr * sin(fTime + vInVertex.x) / 2.0;
	vInNormal = normalize(vInNormal);
#endif
	gl_Position = MVPMatrix * vInVertex;
	
	#ifdef ENABLE_TEXTURE
		TexCoord = inTexCoord;
	#endif
	
	#ifdef ENABLE_FOG_DEPTH
		// Calculate the vertex's distance under water surface. This assumes clipping has removed all objects above the water
		mediump float vVertexHeight = (ModelMatrix * vec4(inVertex,1.0)).y;
		VertexDepth = WaterHeight - vVertexHeight;
	#endif
	
	#ifdef ENABLE_LIGHTING
	    // Simple diffuse lighting in world space
	    lowp vec3 N = normalize((ModelMatrix * vec4(vInNormal, 0.0)).xyz);
	    lowp vec3 L = normalize(LightDirection);
	    LightIntensity = 0.3 + max(0.0, dot(N, -L));
		#ifdef ENABLE_SPECULAR
			LightDir       = L;
			Normal         = N;
	    	EyeDir         = normalize(EyePos - (ModelMatrix * vInVertex).xyz);
    	#endif
	#endif
}
