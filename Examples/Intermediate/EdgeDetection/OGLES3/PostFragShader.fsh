#version 300 es

uniform sampler2D  sColorBufferTexture;	// Texture containing color info in rgb and an object ID value in the alpha channel.
in mediump vec2 t1;				// Texture coordinate for this fragment.

//#define EDGE_DETECTION
//#define INVERSE			These are for editing only, leave disabled as they are passed in by the main program
//#define BLUR

#ifdef EDGE_DETECTION
in mediump vec2 t2;				// Texel directly above this fragment's.
in mediump vec2 t3;				// Texel directly to the right of this fragment's.
#endif

const lowp vec3 black = vec3(0.0);
layout (location = 0) out lowp vec4 oColour;

void main()
{
	// Gets the colour from the render texture.
	lowp vec4 Color = texture(sColorBufferTexture, t1);
		
	// Temporary colour store to be written to, data transferred to gl_FragColor at the end.
	lowp vec3 newColor=Color.rgb;
	
#ifdef EDGE_DETECTION
	// Reads in values from the colour texture, for two surrounding texels;
	lowp vec4 upFrag = texture(sColorBufferTexture, t2);
	lowp vec4 rightFrag = texture(sColorBufferTexture, t3);
	
	// If the object IDs covering this area differ, draw an edge
	if(upFrag.a-Color.a != 0.0 || rightFrag.a-Color.a != 0.0)
	{
	#ifdef INVERSE
		// Sets edge color to inverse of original colour.
		newColor = 1.0-Color.rgb;
	#else 
	#ifdef BLUR
		// Sets edge to a mixture of surrounding colours.
		newColor = (Color.rgb*0.5+(upFrag.rgb+rightFrag.rgb)*0.25);
		
	#else
		// Sets edge to black
		newColor = black;
	#endif
	#endif
	}
	else
	{
	#ifdef INVERSE
		//Sets non-edges to black
		newColor=black;
	#endif
	}
#endif

	// Finally assigns our out colour, with a default alpha value of 1.
	oColour = vec4(newColor,1.0);
}