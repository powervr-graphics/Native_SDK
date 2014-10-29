#version 300 es

#define VERTEX_ARRAY	0
#define NORMAL_ARRAY	1
#define TEXCOORD_ARRAY	2

layout (location = VERTEX_ARRAY) in highp vec3 inVertex;
layout (location = NORMAL_ARRAY) in highp vec3 inNormal;
layout (location = TEXCOORD_ARRAY) in highp vec2 inTexCoord;

#define ENABLE_TEXTURE

#ifdef ENABLE_PERTURB_VTX
    uniform highp float fTime;
#endif

uniform highp mat4		MVPMatrix;
uniform mediump vec3	LightDirection;
uniform highp mat4		ModelMatrix;
#ifdef ENABLE_FOG_DEPTH
uniform mediump float	WaterHeight;		//Assume water always lies on the y-axis
#endif

#ifdef ENABLE_LIGHTING
	out lowp float		LightIntensity;	
	#ifdef ENABLE_SPECULAR
        uniform mediump vec3    EyePos;

        out mediump vec3    EyeDir;
        out mediump vec3    LightDir;
        out mediump vec3    Normal;
    #endif
#endif
#ifdef ENABLE_TEXTURE
	out mediump vec2 	TexCoord;
#endif
#ifdef ENABLE_FOG_DEPTH
	out mediump float	VertexDepth;
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
