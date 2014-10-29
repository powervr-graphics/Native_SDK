attribute mediump vec3 inVertex;

uniform mediump mat4 ModelMatrix;
uniform mediump mat4 ModelViewMatrix;
uniform highp mat4 MVPMatrix;
uniform mediump float WaterHeight;		//Assume water always lies on the y-axis
#ifdef ENABLE_DISCARD_CLIP
uniform bool ClipPlaneBool;
uniform mediump vec4 ClipPlane;
#endif

varying mediump vec3 EyeDir;
varying mediump float VertexHeight;
#ifdef ENABLE_DISCARD_CLIP
varying highp float ClipDist;
#endif

void main()
{
	EyeDir = -inVertex;
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	
	#ifdef ENABLE_DISCARD_CLIP
		// Compute the distance between the vertex and clipping plane (in world space coord system)
		mediump vec4 vVertexView = ModelMatrix * vec4(inVertex.xyz,1.0);
		ClipDist = dot(vVertexView, ClipPlane);
	#endif
	
	// Calculate the vertex's distance ABOVE water surface.
	mediump float vVertexHeight = (ModelMatrix * vec4(inVertex,1.0)).y;
	VertexHeight = vVertexHeight - WaterHeight;
}